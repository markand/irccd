/*
 * log.c -- loggers
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "log.h"

enum level {
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_DEBUG
};

static FILE *out, *err;
static bool verbosity;

static void
handler_files(enum level level, const char *fmt, va_list ap)
{
	switch (level) {
	case LEVEL_WARN:
		vfprintf(err, fmt, ap);
		putc('\n', err);
		fflush(err);
		break;
	default:
		vfprintf(out, fmt, ap);
		putc('\n', out);
		fflush(out);
		break;
	}
}

static void
finalizer_files(void)
{
	if (out)
		fclose(out);
	if (err)
		fclose(err);
}

static void
handler_syslog(enum level level, const char *fmt, va_list ap)
{
	static const int table[] = {
		[LEVEL_INFO] = LOG_INFO,
		[LEVEL_WARN] = LOG_WARNING,
		[LEVEL_DEBUG] = LOG_DEBUG
	};

	/* TODO: add compatibility shim for vsyslog. */
	vsyslog(table[level], fmt, ap);
}

static void
finalizer_syslog(void)
{
	closelog();
}

static void (*handler)(enum level, const char *, va_list);
static void (*finalizer)(void);

void
irc_log_to_syslog(void)
{
	irc_log_finish();

	openlog("irccd", 0, LOG_DAEMON);

	handler = handler_syslog;
	finalizer = finalizer_syslog;
}

void
irc_log_to_console(void)
{
	irc_log_finish();

	out = stdout;
	err = stderr;

	handler = handler_files;
	finalizer = NULL;
}

void
irc_log_to_files(const char *stdout, const char *stderr)
{
	irc_log_finish();

	if (!(out = fopen(stdout, "a")))
		irc_log_warn("%s: %s", stdout, strerror(errno));
	if (!(err = fopen(stderr, "a")))
		irc_log_warn("%s: %s", stdout, strerror(errno));

	handler = handler_files;
	finalizer = finalizer_files;
}

void
irc_log_to_null(void)
{
	irc_log_finish();

	handler = NULL;
	finalizer = NULL;
}

void
irc_log_set_verbose(bool mode)
{
	verbosity = mode;
}

void
irc_log_info(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);

	if (verbosity && handler)
		handler(LEVEL_INFO, fmt, ap);

	va_end(ap);
}

void
irc_log_warn(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);

	if (handler)
		handler(LEVEL_WARN, fmt, ap);

	va_end(ap);
}

void
irc_log_debug(const char *fmt, ...)
{
#if !defined(NDBEUG)
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);

	if (handler)
		handler(LEVEL_DEBUG, fmt, ap);

	va_end(ap);
#else
	(void)fmt;
#endif
}

void
irc_log_finish(void)
{
	if (finalizer)
		finalizer();

	handler = NULL;
	finalizer = NULL;
}
