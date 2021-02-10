/*
 * links.c -- links plugin
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

#include <compat.h>

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

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

static char templates[][512] = {
	[TPL_INFO] = "#{nickname}, voici le lien: #{title}"
};

static size_t
callback(char *ptr, size_t size, size_t nmemb, struct req *req)
{
	fprintf(req->fp, "%.*s", (int)(size * nmemb), ptr);

	if (feof(req->fp) || ferror(req->fp))
		return 0;

	return size * nmemb;
}

static const char *
parse(struct req *req)
{
	regex_t regex;
	regmatch_t match[2];
	char *ret = NULL;

	if (regcomp(&regex, "<title>([^<]+)<\\/title>",	REG_EXTENDED | REG_ICASE) != 0)
		return NULL;

	if (regexec(&regex, req->buf, 2, match, 0) == 0) {
		ret = &req->buf[match[1].rm_so];
		ret[match[1].rm_eo - match[1].rm_so] = '\0';
	}

	regfree(&regex);

	return ret;
}

static const char *
fmt(const struct req *req, const char *title)
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
			{ "title",      title                   }
		},
		.keywordsz = 5
	};

	irc_subst(line, sizeof (line), templates[TPL_INFO], &subst);

	return line;
}

static void
req_finish(struct req *);

static void
complete(struct req *req)
{
	const char *title;

	if (req->status && (title = parse(req)))
		irc_server_message(req->server, req->chan, fmt(req, title));

	pthread_join(req->thr, NULL);
	req_finish(req);
}

/*
 * This function is running in a separate thread.
 */
static void *
routine(struct req *req)
{
	typedef void (*func_t)(void *);

	if (curl_easy_perform(req->curl) == 0)
		req->status = 1;

	irc_bot_post((func_t)complete, req);

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

	return req;

enomem:
	errno = ENOMEM;
	req_finish(req);

	return NULL;
}

static void
req_start(struct req *req)
{
	typedef void *(*func_t)(void *);

	if (pthread_create(&req->thr, NULL, (func_t)routine, req) != 0)
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
		strlcpy(templates[TPL_INFO], value, sizeof (templates[TPL_INFO]));
}

const char *
links_get_template(const char *key)
{
	if (strcmp(key, "info") == 0)
		return templates[TPL_INFO];

	return NULL;
}

const char **
links_get_templates(void)
{
	static const char *keys[] = { "info", NULL };

	return keys;
}

const char *links_description = "Parse links from messages";
const char *links_version = IRCCD_VERSION;
const char *links_license = "ISC";
const char *links_author = "David Demelier <markand@malikania.fr>";
