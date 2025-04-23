/*
 * util.h -- miscellaneous utilities
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

#ifndef IRCCD_UTIL_H
#define IRCCD_UTIL_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define IRC_UTIL_SIZE(x) (sizeof (x) / sizeof (x[0]))

/* Suitable convenient typedef for bsearch/qsort. */
typedef int (*irc_cmp)(const void *, const void *);

void *
irc_util_malloc(size_t);

void *
irc_util_calloc(size_t, size_t);

void *
irc_util_realloc(void *, size_t);

void *
irc_util_reallocarray(void *, size_t, size_t);

void *
irc_util_memdup(const void *, size_t);

char *
irc_util_strdup(const char *);

char *
irc_util_strndup(const char *, size_t);

char *
irc_util_basename(const char *);

char *
irc_util_dirname(const char *);

size_t
irc_util_split(char *, const char **, size_t, char);

char *
irc_util_printf(char *, size_t, const char *, ...);

void
irc_util_die(const char *, ...);

int
irc_util_stoi(const char *, long long *);

int
irc_util_stou(const char *, unsigned long long *);

void *
irc_util_reallocarray(void *, size_t, size_t);

size_t
irc_util_strlcpy(char *, const char *, size_t);

size_t
irc_util_strlcat(char *, const char *, size_t);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_UTIL_H */
