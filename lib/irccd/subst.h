/*
 * subst.h -- pattern substitution
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

#ifndef IRCCD_SUBST_H
#define IRCCD_SUBST_H

/**
 * \file subst.h
 * \brief Pattern substitution.
 */

#include <sys/types.h>
#include <stddef.h>
#include <time.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \brief Substitution flags.
 */
enum irc_subst_flags {
	/**
	 * Allow date and time substitution.
	 */
	IRC_SUBST_DATE = (1 << 0),

	/**
	 * Allow keyword based substitution.
	 */
	IRC_SUBST_KEYWORDS = (1 << 1),

	/**
	 * Allow system environment variable substitution.
	 *
	 * \warning Use this with care
	 */
	IRC_SUBST_ENV = (1 << 2),

	/**
	 * Allow system shell variable execution from commands.
	 *
	 * \warning Use this with extreme care
	 */
	IRC_SUBST_SHELL = (1 << 3),

	/**
	 * Allow special IRC attributes substitution for color and style.
	 */
	IRC_SUBST_IRC_ATTRS = (1 << 4),

	/**
	 * Allow shell attribute (escape sequence) for color and style.
	 */
	IRC_SUBST_SHELL_ATTRS = (1 << 5)
};

/**
 * \brief Key-value table of keyword substitution.
 */
struct irc_subst_keyword {
	/**
	 * (read-only, borrowed)
	 *
	 * Keyword name.
	 */
	const char *key;

	/**
	 * (read-only, borrowed)
	 *
	 * Keyword replacement.
	 */
	const char *value;
};

/**
 * \brief Substitution parameters.
 */
struct irc_subst {
	/**
	 * (read-write)
	 *
	 * Flags to allow for substitution.
	 */
	enum irc_subst_flags flags;

	/**
	 * (read-write)
	 *
	 * Timestamp to use if using ::IRC_SUBST_DATE.
	 */
	time_t time;

	/**
	 * (read-write, borrowed, optional)
	 *
	 * Table of keywords to replace if using ::IRC_SUBST_KEYWORDS.
	 */
	const struct irc_subst_keyword *keywords;

	/**
	 * (read-write)
	 *
	 * Number of elements in ::irc_subst::keywords if not NULL.
	 */
	size_t keywordsz;
};

/**
 * Perform a template substitution.
 *
 * \pre out != NULL
 * \pre in != NULL
 * \param out the output string
 * \param outsz maximum number of bytes to write in out
 * \param in the template string
 * \param subst the substitution parameters
 * \return the number of bytes written (excluding NUL) or -1 on error
 */
ssize_t
irc_subst(char *out, size_t outsz, const char *in, const struct irc_subst *subst);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_SUBST_H */
