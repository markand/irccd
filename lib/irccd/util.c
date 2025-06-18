/*
 * util.c -- miscellaneous utilities
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define REGEX_USER "([^!]+)!([^@]+)@(.*)"

void *
irc_util_malloc(size_t size)
{
	void *ret;

	if (!(ret = malloc(size)))
		irc_util_die("malloc: %s\n", strerror(errno));

	return ret;
}

void *
irc_util_calloc(size_t n, size_t size)
{
	void *ret;

	if (!(ret = calloc(n, size)))
		irc_util_die("calloc: %s\n", strerror(errno));

	return ret;
}

void *
irc_util_realloc(void *ptr, size_t size)
{
	void *ret;

	if (!(ret = realloc(ptr, size)) && size)
		irc_util_die("realloc: %s\n", strerror(errno));

	return ret;
}

void *
irc_util_memdup(const void *ptr, size_t size)
{
	assert(ptr);

	return memcpy(irc_util_malloc(size), ptr, size);
}

void *
irc_util_free(void *ptr)
{
	free(ptr);

	return NULL;
}

char *
irc_util_strdup(const char *src)
{
	char *ret;

	if (!(ret = strdup(src)))
		irc_util_die("strdup: %s\n", strerror(errno));

	return ret;
}

char *
irc_util_strndup(const char *src, size_t n)
{
	assert(src);

	char *ret;

	if (!(ret = strndup(src, n)))
		irc_util_die("strndup: %s\n", strerror(errno));

	return ret;
}

char *
irc_util_strdupfree(char *ptr, const char *value)
{
	/*
	 * Avoid reallocating if the new value is the original pointer. This may
	 * happen when user is trying to use default values and does not want to
	 * check for convenience.
	 */
	if (ptr == value)
		return ptr;

	free(ptr);

	if (value)
		ptr = irc_util_strdup(value);
	else
		ptr = NULL;

	return ptr;
}

char *
irc_util_basename(const char *str)
{
	static char ret[PATH_MAX];
	char tmp[PATH_MAX];

	irc_util_strlcpy(tmp, str, sizeof (tmp));
	irc_util_strlcpy(ret, basename(tmp), sizeof (ret));

	return ret;
}

char *
irc_util_dirname(const char *str)
{
	static char ret[PATH_MAX];
	char tmp[PATH_MAX];

	irc_util_strlcpy(tmp, str, sizeof (tmp));
	irc_util_strlcpy(ret, dirname(tmp), sizeof (ret));

	return ret;
}

size_t
irc_util_split(char *line, const char **args, size_t max, char delim)
{
	size_t idx;

	if (!*line)
		return 0;

	for (idx = 0; idx < max; ++idx) {
		char *sp = strchr(line, delim);

		if (!sp || idx + 1 >= max) {
			args[idx++] = line;
			break;
		}

		*sp = '\0';
		args[idx] = line;
		line = sp + 1;
	}

	return idx;
}

char *
irc_util_printf(char *buf, size_t bufsz, const char *fmt, ...)
{
	assert(buf);
	assert(bufsz);
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, bufsz, fmt, ap);
	va_end(ap);

	return buf;
}

void
irc_util_die(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

int
irc_util_stoi(const char *s, long long *i)
{
	assert(s);
	assert(i);

	errno = 0;
	*i = strtoll(s, NULL, 10);

	return errno == 0 ? 0 : -1;
}

int
irc_util_stou(const char *s, unsigned long long *u)
{
	assert(s);
	assert(u);

	errno = 0;
	*u = strtoull(s, NULL, 10);

	return errno == 0 ? 0 : -1;
}

void *
irc_util_reallocarray(void *ptr, size_t n, size_t w)
{
	return reallocarray(ptr, n, w);
}

size_t
irc_util_strlcpy(char *dst, const char *src, size_t len)
{
	assert(dst);

	return strlcpy(dst, src, len);
}

size_t
irc_util_strlcat(char *dst, const char *src, size_t len)
{
	assert(dst);

	return strlcat(dst, src, len);
}

struct irc_user *
irc_util_user_split(const char *prefix)
{
	assert(prefix);

	struct irc_user *user = NULL;
	regex_t regex;
	regmatch_t matches[4] = {};

	if (regcomp(&regex, REGEX_USER, REG_EXTENDED) != 0)
		return NULL;

	user = irc_util_calloc(1, sizeof (*user));

	if (regexec(&regex, prefix, IRC_UTIL_SIZE(matches), matches, 0) == 0) {
		user->nickname = irc_util_strndup(&prefix[matches[1].rm_so], matches[1].rm_eo - matches[1].rm_so);
		user->username = irc_util_strndup(&prefix[matches[2].rm_so], matches[2].rm_eo - matches[2].rm_so);
		user->host     = irc_util_strndup(&prefix[matches[3].rm_so], matches[3].rm_eo - matches[3].rm_so);
	} else
		user->nickname = irc_util_strdup(prefix);

	regfree(&regex);

	return user;
}

void
irc_util_user_free(struct irc_user *user)
{
	free(user->nickname);
	free(user->username);
	free(user->host);
	free(user);
}
