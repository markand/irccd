/*
 * links.c -- links plugin
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include <irccd/config.h>
#include <irccd/irccd.h>
#include <irccd/limits.h>
#include <irccd/server.h>
#include <irccd/subst.h>
#include <irccd/util.h>

/*
 * Since most websites are now bloated, we need a very large page size to
 * analyse. Use 5MB for now.
 */
#define PAGE_MAX 5242880

struct req {
	pthread_t thr;
	CURL *curl;
	struct irc_server *server;
	int status;
	char *link;
	char *chan;
	char *nickname;
	char *origin;
	FILE *fp;
	char buf[];
};

enum {
	TPL_INFO
};

static unsigned long timeout = 30;

static char templates[][512] = {
	[TPL_INFO] = "link from #{nickname}: #{title}"
};

static const struct {
	const char *key;
	int repl;
} entities[] = {
	{ "quot",       '"'     },
	{ "amp",        '&'     },
	{ "apos",       '\''    },
	{ "lt",         '<'     },
	{ "gt",         '>'     },
	{ NULL,         0       }
};

static size_t
callback(char *ptr, size_t size, size_t nmemb, struct req *req)
{
	fprintf(req->fp, "%.*s", (int)(size * nmemb), ptr);

	if (feof(req->fp) || ferror(req->fp))
		return 0;

	return size * nmemb;
}

static char *
parse(struct req *req)
{
	regex_t regex;
	regmatch_t match[2];
	char *ret = NULL;

	if (regcomp(&regex, "<title>([^<]+)<\\/title>", REG_EXTENDED | REG_ICASE) != 0)
		return NULL;

	if (regexec(&regex, req->buf, 2, match, 0) == 0) {
		ret = &req->buf[match[1].rm_so];
		ret[match[1].rm_eo - match[1].rm_so] = '\0';
	}

	regfree(&regex);

	return ret;
}

static int
find_entity(const char *key)
{
	for (size_t i = 0; entities[i].key; ++i)
		if (strcmp(entities[i].key, key) == 0)
			return entities[i].repl;

	return EOF;
}

static const char *
untitle(char *title)
{
	static char ret[256] = {0};
	char *save;
	int repl;
	FILE *fp = NULL;

	if (!(fp = fmemopen(ret, sizeof (ret) - 1, "w")))
		goto fallback;

	for (char *p = title; *p; ) {
		/* Standard character. */
		if (*p != '&') {
			if (fputc(*p++, fp) == EOF)
				goto fallback;

			continue;
		}

		/* HTML entity. */
		save = ++p;

		while (*p && *p != ';')
			++p;

		/* Found an entity. */
		if (*p == ';') {
			*p++ = '\0';

			if ((repl = find_entity(save)) != EOF)
				fputc(repl, fp);
		}
	}

	fclose(fp);

	return ret;

fallback:
	if (fp)
		fclose(fp);

	return title;
}

static const char *
fmt(const struct req *req, char *title)
{
	static char line[IRC_MESSAGE_LEN];

	struct irc_subst subst = {
		.time = time(NULL),
		.flags = IRC_SUBST_DATE | IRC_SUBST_KEYWORDS | IRC_SUBST_IRC_ATTRS,
		.keywords = (struct irc_subst_keyword []) {
			{ "channel",    req->chan               },
			{ "nickname",   req->nickname           },
			{ "origin",     req->origin             },
			{ "server",     req->server->name       },
			{ "title",      untitle(title)          }
		},
		.keywordsz = 5
	};

	irc_subst(line, sizeof (line), templates[TPL_INFO], &subst);

	return line;
}

static void
req_finish(struct req *);

static void
complete(void *data)
{
	struct req *req = data;
	char *title;

	if (req->status && (title = parse(req)))
		irc_server_message(req->server, req->chan, fmt(req, title));

	pthread_join(req->thr, NULL);
	req_finish(req);
}

/*
 * This function is running in a separate thread.
 */
static void *
routine(void *data)
{
	struct req *req = data;
	long code = 0;

	if (curl_easy_perform(req->curl) == 0) {
		/* We only accept 200 result. */
		curl_easy_getinfo(req->curl, CURLINFO_RESPONSE_CODE, &code);
		req->status = code == 200;
	}

	irc_bot_post(complete, req);

	return NULL;
}

static void
req_finish(struct req *req)
{
	assert(req);

	if (req->server)
		irc_server_decref(req->server);
	if (req->curl)
		curl_easy_cleanup(req->curl);
	if (req->fp)
		fclose(req->fp);

	free(req->link);
	free(req->chan);
	free(req->nickname);
	free(req->origin);
	free(req);
}

static struct req *
req_new(struct irc_server *server, const char *origin, const char *channel, char *link)
{
	assert(link);

	struct req *req;
	struct irc_server_user user;

	if (!(req = calloc(1, sizeof (*req) + PAGE_MAX + 1))) {
		free(link);
		return NULL;
	}
	if (!(req->curl = curl_easy_init()))
		goto enomem;
	if (!(req->fp = fmemopen(req->buf, PAGE_MAX, "w")))
		goto enomem;

	irc_server_incref(server);
	irc_server_split(origin, &user);
	req->link = link;
	req->server = server;
	req->chan = strdup(channel);
	req->nickname = strdup(user.nickname);
	req->origin = strdup(origin);

	curl_easy_setopt(req->curl, CURLOPT_URL, link);
	curl_easy_setopt(req->curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(req->curl, CURLOPT_WRITEFUNCTION, callback);
	curl_easy_setopt(req->curl, CURLOPT_WRITEDATA, req);
	curl_easy_setopt(req->curl, CURLOPT_TIMEOUT, timeout);

	return req;

enomem:
	errno = ENOMEM;
	req_finish(req);

	return NULL;
}

static void
req_start(struct req *req)
{
	if (pthread_create(&req->thr, NULL, routine, req) != 0)
		req_finish(req);
}

void
links_event(const struct irc_event *ev)
{
	struct req *req;
	char *loc, *link, *end;

	if (ev->type != IRC_EVENT_MESSAGE)
		return;

	/* Parse link. */
	if (!(loc = strstr(ev->message.message, "http://")) &&
	    !(loc = strstr(ev->message.message, "https://")))
	    return;

	/* Compute end to allocate only what's needed. */
	for (end = loc; *end && !isspace(*end); )
		++end;

	link = irc_util_strndup(loc, end - loc);

	/* If the function fails, link is free'd anyway. */
	if (!(req = req_new(ev->server, ev->message.origin, ev->message.channel, link)))
		return;

	req_start(req);
}

void
links_set_template(const char *key, const char *value)
{
	if (strcmp(key, "info") == 0)
		irc_util_strlcpy(templates[TPL_INFO], value, sizeof (templates[TPL_INFO]));
}

const char *
links_get_template(const char *key)
{
	if (strcmp(key, "info") == 0)
		return templates[TPL_INFO];

	return NULL;
}

const char * const *
links_get_templates(void)
{
	static const char *keys[] = { "info", NULL };

	return keys;
}

void
links_set_option(const char *key, const char *value)
{
	if (strcmp(key, "timeout") == 0)
		timeout = atol(value);
}

const char *
links_get_option(const char *key)
{
	static char out[32];

	if (strcmp(key, "timeout") == 0) {
		snprintf(out, sizeof (out), "%lu", timeout);
		return out;
	}

	return NULL;
}

const char * const *
links_get_options(void)
{
	static const char *keys[] = { "timeout", NULL };

	return keys;
}

const char *links_description = "Parse links from messages";
const char *links_version = IRCCD_VERSION;
const char *links_license = "ISC";
const char *links_author = "David Demelier <markand@malikania.fr>";
