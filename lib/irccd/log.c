/*
 * log.c -- loggers
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

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "log.h"
#include "subst.h"
#include "util.h"

#define DEFAULT_TEMPLATE "#{message}"

enum level {
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_DEBUG
};

static FILE *out, *err;
static int verbosity;
static char tmpl[512] = DEFAULT_TEMPLATE;

static void
handler_files(enum level level, const char *line)
{
	switch (level) {
	case LEVEL_WARN:
		fprintf(err, "%s\n", line);
		fflush(err);
		break;
	default:
		fprintf(out, "%s\n", line);
		fflush(out);
		break;
	}
}

static void
finalizer_files(void)
{
	/* Out and err are the same. */
	if (out)
		fclose(out);

	out = err = NULL;
}

static void
handler_syslog(enum level level, const char *line)
{
	static const int table[] = {
		[LEVEL_INFO] = LOG_INFO,
		[LEVEL_WARN] = LOG_WARNING,
		[LEVEL_DEBUG] = LOG_DEBUG
	};

	syslog(table[level], "%s", line);
}

static void
finalizer_syslog(void)
{
	closelog();
}

static void (*handler)(enum level, const char *);
static void (*finalizer)(void);

static const char *levelstr[] = {
	[LEVEL_DEBUG]   = "debug",
	[LEVEL_INFO]    = "info",
	[LEVEL_WARN]    = "warning"
};

static void
wrap(enum level level, const char *fmt, va_list ap)
{
	char formatted[1024] = {}, line[1024] = {};
	struct irc_subst_keyword kw[] = {
		{ "message",    line            },
		{ "level",      levelstr[level] }
	};
	struct irc_subst subst = {
		.time = time(NULL),
		.flags = IRC_SUBST_DATE |
		         IRC_SUBST_KEYWORDS |
		         IRC_SUBST_ENV |
		         IRC_SUBST_SHELL |
		         IRC_SUBST_SHELL_ATTRS,
		.keywords = kw,
		.keywordsz = IRC_UTIL_SIZE(kw)
	};

	vsnprintf(line, sizeof (line), fmt, ap);
	irc_subst(formatted, sizeof (formatted), tmpl, &subst);
	handler(level, formatted);
}

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
irc_log_to_file(const char *file)
{
	irc_log_finish();

	if (!(out = err = fopen(file, "a")))
		irc_log_warn("%s: %s", file, strerror(errno));

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
irc_log_set_verbose(int mode)
{
	verbosity = mode;
}

void
irc_log_set_template(const char *fmt)
{
	irc_util_strlcpy(tmpl, fmt ? fmt : DEFAULT_TEMPLATE, sizeof (tmpl));
}

void
irc_log_info(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);

	if (verbosity && handler)
		wrap(LEVEL_INFO, fmt, ap);

	va_end(ap);
}

void
irc_log_warn(const char *fmt, ...)
{
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);

	if (handler)
		wrap(LEVEL_WARN, fmt, ap);

	va_end(ap);
}

void
irc_log_debug(const char *fmt, ...)
{
#if !defined(NDEBUG)
	assert(fmt);

	va_list ap;

	va_start(ap, fmt);

	if (handler)
		wrap(LEVEL_DEBUG, fmt, ap);

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
