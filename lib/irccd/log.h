/*
 * log.h -- loggers
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

#ifndef IRCCD_LOG_H
#define IRCCD_LOG_H

#if defined(__cplusplus)
extern "C" {
#endif

void
irc_log_to_syslog(void);

void
irc_log_to_console(void);

void
irc_log_to_file(const char *);

void
irc_log_to_null(void);

void
irc_log_set_verbose(int);

void
irc_log_set_template(const char *);

void
irc_log_info(const char *, ...);

void
irc_log_warn(const char *, ...);

void
irc_log_debug(const char *, ...);

void
irc_log_finish(void);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_LOG_H */
