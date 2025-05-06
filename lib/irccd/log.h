/*
 * log.h -- logging API
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

#ifndef IRCCD_LOG_H
#define IRCCD_LOG_H

/**
 * \file log.h
 * \brief Logging API.
 */

#include "attrs.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Setup logging to syslog.
 */
void
irc_log_to_syslog(void);

/**
 * Setup logging to console.
 */
void
irc_log_to_console(void);

/**
 * Setup logging to a file.
 *
 * \pre path != NULL
 * \param path the filename to logs
 */
void
irc_log_to_file(const char *path);

/**
 * Disable logging entirely.
 */
void
irc_log_to_null(void);

/**
 * Change logging verbosity.
 *
 * \param mode set to non-zero to be more verbose
 */
void
irc_log_set_verbose(int mode);

/**
 * Change the template format for logging.
 *
 * The logging format supports:
 *
 * - date
 * - environment
 * - keywords: #{message}, #{level}
 * - shell
 * - shell attributes
 *
 * Passing NULL as argument reset to the default template.
 *
 * \param format the format template (may be NULL)
 */
void
irc_log_set_template(const char *format);

/**
 * Write a general information message if verbosity is enabled.
 *
 * \pre fmt != NULL
 * \param fmt the printf(3) format style
 */
IRC_ATTR_PRINTF(1, 2)
void
irc_log_info(const char *fmt, ...);

/**
 * Write a warning message.
 *
 * \pre fmt != NULL
 * \param fmt the printf(3) format style
 */
IRC_ATTR_PRINTF(1, 2)
void
irc_log_warn(const char *fmt, ...);

/**
 * Write a debug message.
 *
 * The message will only be shown when irccd is build in debug mode and no
 * matter if verbosity is set or not.
 *
 * \pre fmt != NULL
 * \param fmt the printf(3) format style
 */
IRC_ATTR_PRINTF(1, 2)
void
irc_log_debug(const char *fmt, ...);

/**
 * Close the opened logger.
 */
void
irc_log_finish(void);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_LOG_H */
