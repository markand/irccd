/*
 * jsapi-util.c -- Irccd.Util API
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

#include <string.h>

#include <irccd/server.h>
#include <irccd/subst.h>
#include <irccd/util.h>

#include "jsapi-util.h"

struct subspack {
	struct irc_subst_keyword *kw;
	struct irc_subst subst;
};

struct string {
	TAILQ_ENTRY(string) link;
	char value[];
};

TAILQ_HEAD(stringlist, string);

static inline void
subspack_finish(struct subspack *subst)
{
	for (size_t i = 0; i < subst->subst.keywordsz; ++i) {
		free((char *)subst->kw[i].key);
		free((char *)subst->kw[i].value);
	}

	free(subst->kw);
}

/*
 * Read parameters for Irccd.Util.format function, the object is defined as
 * following:
 *
 * {
 *   date: the date object
 *   flags: the flags (not implemented yet)
 *   field1: a field to substitute in #{} pattern
 *   field2: a field to substitute in #{} pattern
 *   fieldn: ...
 * }
 */
static void
subspack_parse(duk_context *ctx, duk_idx_t index, struct subspack *pkg)
{
	memset(pkg, 0, sizeof (*pkg));

	if (!duk_is_object(ctx, index))
		return;

	/* Use current time by default. */
	pkg->subst.time = time(NULL);

	duk_enum(ctx, index, 0);

	while (duk_next(ctx, -1, 1)) {
		if (!duk_is_string(ctx, -2)) {
			subspack_finish(pkg);
			(void)duk_error(ctx, DUK_ERR_TYPE_ERROR, "keyword name must be a string");
		}

		if (strcmp(duk_get_string(ctx, -2), "date") == 0)
			pkg->subst.time = duk_get_number(ctx, -1) / 1000;
		else {
			pkg->kw = irc_util_reallocarray(pkg->kw, ++pkg->subst.keywordsz,
			    sizeof (*pkg->kw));
			pkg->kw[pkg->subst.keywordsz - 1].key =
			    irc_util_strdup(duk_get_string_default(ctx, -2, ""));
			pkg->kw[pkg->subst.keywordsz - 1].value =
			    irc_util_strdup(duk_get_string_default(ctx, -1, ""));
		}

		duk_pop_n(ctx, 2);
	}

	pkg->subst.flags = IRC_SUBST_DATE |
	                   IRC_SUBST_KEYWORDS |
	                   IRC_SUBST_ENV |
	                   IRC_SUBST_IRC_ATTRS;
	pkg->subst.keywords = pkg->kw;
}

static struct string *
string_new(const char *v)
{
	struct string *s;
	const size_t len = strlen(v);

	s = irc_util_malloc(sizeof (*s) + len + 1);
	strcpy(s->value, v);

	return s;
}

static void
stringlist_finish(struct stringlist *list)
{
	struct string *s, *tmp;

	TAILQ_FOREACH_SAFE(s, list, link, tmp)
		free(s);
}

static void
stringlist_concat(struct stringlist *list, const char *value)
{
	struct string *s;
	char *str = irc_util_strdup(value), *token, *p = str;

	while ((token = strtok_r(p, " \t\n", &p))) {
		/* TODO: trim and check if empty. */
		s = string_new(token);
		TAILQ_INSERT_TAIL(list, s, link);
	}

	free(str);
}

static void
split_from_string(duk_context *ctx, struct stringlist *list)
{
	stringlist_concat(list, duk_require_string(ctx, 0));
}

static void
split_from_array(duk_context *ctx, struct stringlist *list)
{
	duk_enum(ctx, 0, DUK_ENUM_ARRAY_INDICES_ONLY);

	while (duk_next(ctx, -1, 1)) {
		stringlist_concat(list, duk_to_string(ctx, -1));
		duk_pop_2(ctx);
	}
}

static void
split(duk_context *ctx, duk_idx_t index, struct stringlist *list)
{
	duk_require_type_mask(ctx, index, DUK_TYPE_MASK_OBJECT | DUK_TYPE_MASK_STRING);
	TAILQ_INIT(list);

	if (duk_is_string(ctx, index))
		split_from_string(ctx, list);
	else if (duk_is_array(ctx, index))
		split_from_array(ctx, list);
}

static int
limit(duk_context *ctx, duk_idx_t index, const char *name, size_t value)
{
	int newvalue;

	if (duk_get_top(ctx) < index || !duk_is_number(ctx, index))
		return value;

	newvalue = duk_to_int(ctx, index);

	if (newvalue <= 0)
		(void)duk_error(ctx, DUK_ERR_RANGE_ERROR,
		    "argument %d (%s) must be positive", index, name);

	return newvalue;
}

static char *
join(duk_context *ctx, size_t maxc, size_t maxl, const struct stringlist *tokens)
{
	FILE *fp;
	char *out = NULL;
	size_t outsz = 0, linesz = 0, tokensz, lineavail = maxl;
	struct string *token;

	if (!(fp = open_memstream(&out, &outsz)))
		return NULL;

	TAILQ_FOREACH(token, tokens, link) {
		tokensz = strlen(token->value);

		if (tokensz > maxc) {
			fclose(fp);
			duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR,
			    "token '%s' could not fit in maxc limit (%zu)", token->value, maxc);
			return NULL;
		}

		/*
		 * If there is something at the beginning of the line, we must
		 * append a space.
		 */
		if (linesz > 0)
			tokensz++;

		/*
		 * This token is going past the maximum of the current line so
		 * we append a newline character and reset the length to start
		 * a "new" one.
		 */
		if (linesz + tokensz > maxc) {
			if (--lineavail == 0) {
				fclose(fp);
				duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR, "number of lines exceeds maxl (%zu)", maxl);
				return NULL;
			}

			fputc('\n', fp);
			linesz = 0;
		}

		linesz += fprintf(fp, "%s%s", linesz > 0 ? " " : "", token->value);
	}

	fflush(fp);
	fclose(fp);

	return out;
}

static int
Util_cut(duk_context *ctx)
{
	struct stringlist tokens;
	size_t maxc, maxl, i = 0;
	char *lines, *line, *p;

	maxc = limit(ctx, 1, "maxc", 72);
	maxl = limit(ctx, 2, "maxl", SIZE_MAX);

	/* Construct a list of words from a string or an array of strings.  */
	split(ctx, 0, &tokens);

	/* Join as new lines with a limit of maximum columns and lines. */
	if (!(lines = join(ctx, maxc, maxl, &tokens))) {
		stringlist_finish(&tokens);
		return duk_throw(ctx);
	}

	duk_push_array(ctx);

	for (p = lines; (line = strtok_r(p, "\n", &p)); ) {
		duk_push_string(ctx, line);
		duk_put_prop_index(ctx, -2, i++);
	}

	stringlist_finish(&tokens);
	free(lines);

	return 1;
}

static int
Util_format(duk_context *ctx)
{
	const char *str = duk_require_string(ctx, 0);
	struct subspack pkg;
	char buf[1024] = {0};

	subspack_parse(ctx, 1, &pkg);
	irc_subst(buf, sizeof (buf), str, &pkg.subst);
	duk_push_string(ctx, buf);
	subspack_finish(&pkg);

	return 1;
}

static int
Util_splituser(duk_context *ctx)
{
	struct irc_server_user user;

	irc_server_split(duk_require_string(ctx, 0), &user);
	duk_push_string(ctx, user.nickname);

	return 1;
}

static int
Util_splithost(duk_context *ctx)
{
	struct irc_server_user user;

	irc_server_split(duk_require_string(ctx, 0), &user);
	duk_push_string(ctx, user.host);

	return 1;
}

static const duk_function_list_entry functions[] = {
	{ "cut",        Util_cut,       DUK_VARARGS     },
	{ "format",     Util_format,    DUK_VARARGS     },
	{ "splituser",  Util_splituser, 1               },
	{ "splithost",  Util_splithost, 1               },
	{ NULL,         NULL,           0               }
};

void
jsapi_util_load(duk_context *ctx)
{
	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "Util");
	duk_pop(ctx);
}
