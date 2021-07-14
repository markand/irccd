/*
 * util.c -- miscellaneous utilities
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

#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

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
irc_util_reallocarray(void *ptr, size_t n, size_t size)
{
	void *ret;

	if (!(ret = reallocarray(ptr, n, size)))
		irc_util_die("reallocarray: %s\n", strerror(errno));

	return ret;
}

void *
irc_util_memdup(const void *ptr, size_t size)
{
	assert(ptr);

	return memcpy(irc_util_malloc(size), ptr, size);
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
irc_util_basename(const char *str)
{
	static char ret[PATH_MAX];
	char tmp[PATH_MAX];

	strlcpy(tmp, str, sizeof (tmp));
	strlcpy(ret, basename(tmp), sizeof (ret));

	return ret;
}

char *
irc_util_dirname(const char *str)
{
	static char ret[PATH_MAX];
	char tmp[PATH_MAX];

	strlcpy(tmp, str, sizeof (tmp));
	strlcpy(ret, dirname(tmp), sizeof (ret));

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

noreturn void
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
