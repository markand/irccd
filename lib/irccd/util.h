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

#include "attrs.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define IRC_UTIL_CONTAINER_OF(Ptr, Type, Member) \
        ((Type *)((char *)(1 ? (Ptr) : &((Type *)0)->Member) - offsetof(Type, Member)))

#define IRC_UTIL_SIZE(x) (sizeof (x) / sizeof (x[0]))

struct irc_user {
	char *nickname;
	char *username;
	char *host;
};

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

/**
 * Convenient function calling free(ptr) and returning NULL.
 *
 * This function is designed to cleanup resources while restoring pointers to
 * NULL values.
 *
 * \param ptr the pointer to free (may be NULL)
 * \return NULL
 */
void *
irc_util_free(void *ptr);

char *
irc_util_strdup(const char *);

char *
irc_util_strndup(const char *, size_t);

/**
 * Duplicate the string `value` if not NULL free'ing the first argument `ptr`
 * first.
 *
 * This function is designed to "replace" dynamically allocated string with ease
 * with the convenience of allowing NULL values.
 *
 * Example:
 *
 * ```c
 * address = irc_util_strdupfree(address, "my new address value");
 * ```
 *
 * \param ptr the pointer to free (may be NULL)
 * \param value the value to duplicate (may be NULL)
 * \return a newly allocated string or NULL if value was also NULL
 */
char *
irc_util_strdupfree(char *ptr, const char *value);

char *
irc_util_basename(const char *);

char *
irc_util_dirname(const char *);

size_t
irc_util_split(char *, const char **, size_t, char);

IRC_ATTR_PRINTF(3, 4)
char *
irc_util_printf(char *, size_t, const char *, ...);

IRC_ATTR_PRINTF(1, 2)
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

struct irc_user *
irc_util_user_split(const char *ident);

void
irc_util_user_free(struct irc_user *user);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_UTIL_H */
