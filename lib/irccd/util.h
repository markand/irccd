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

/**
 * \brief Describe a IRC user.
 *
 * This structure holds information of a IRC user split from a string in the form
 * `nickname!username@hostname`.
 */
struct irc_user {
	/**
	 * (read-only)
	 *
	 * The `nickname` part from the user ident.
	 */
	char *nickname;

	/**
	 * (read-only)
	 *
	 * The `username` part from the user ident.
	 */
	char *username;

	/**
	 * (read-only)
	 *
	 * The `hostname` part from the user ident.
	 */
	char *host;
};

/**
 * Wrapper to `malloc()` exiting on failure.
 */
void *
irc_util_malloc(size_t size);

/**
 * Wrapper to `calloc()` exiting on failure.
 */
void *
irc_util_calloc(size_t n, size_t w);

/**
 * Wrapper to `realloc()` exiting on failure.
 */
void *
irc_util_realloc(void *ptr, size_t size);

/**
 * Wrapper to OpenBSD/C23 `reallocarray()` exiting on failure.
 */
void *
irc_util_reallocarray(void *ptr, size_t n, size_t w);

/**
 * Create a copy of the memory region, exiting on allocation failure.
 *
 * \pre ptr != NULL
 * \param ptr the pointer to copy
 * \param size the object size
 * \return a dynamically allocated memory to be free'd
 */
void *
irc_util_memdup(const void *ptr, size_t size);

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

/**
 * Wrapper to `strdup()` exiting on failure.
 */
char *
irc_util_strdup(const char *src);

/**
 * Wrapper to `strndup()` exiting on failure.
 */
char *
irc_util_strndup(const char *src, size_t limit);

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

/**
 * Convenient basename(3) alternative that do not modify input.
 *
 * \note the returned string points to a static storage
 * \pre path != NULL
 * \param path the path to decompose
 * \return the filename from path
 */
char *
irc_util_basename(const char *);

/**
 * Convenient dirname(3) alternative that do not modify input.
 *
 * \note the returned string points to a static storage
 * \pre path != NULL
 * \param path the path to decompose
 * \return the directory parent from path
 */
char *
irc_util_dirname(const char *);

/**
 * Split a string into individual components without any dynamic allocation.
 *
 * This function modifies `src`, changing each `delim` character found to '\0',
 * then the `list` parameter is updated to point to the beginning of the new
 * token.
 *
 * This, the `src` argument must remain valid until the `list` is no longer
 * used.
 *
 * \pre src != NULL
 * \param src the source string to split
 * \param list the array of tokens to be set
 * \param max the maximum numbers of items that can be set in list
 * \param delim the character delimiter used to split
 * \return the number of tokens found
 */
size_t
irc_util_split(char *src, const char **list, size_t max, char delim);

#if defined(DOXYGEN)

/**
 * Convenient function that is similar to `snprintf()` except that it returns
 * the string addres to be used directly as a function call.
 *
 * \pre buf != NULL
 * \pre fmt != NULL
 * \param buf the destination string
 * \param bufsz the maximum number to write in buf
 * \param fmt the printf(3) format string
 * \return buf pointer
 */
char *
irc_util_printf(char *buf, size_t bufsz, const char *fmt, ...);

#else

IRC_ATTR_PRINTF(3, 4)
char *
irc_util_printf(char *, size_t, const char *, ...);

#endif

#if defined(DOXYGEN)

/**
 * Write a message to stderr and exit with error code 1.
 */
void
irc_util_die(const char *fmt, ...);

#else

IRC_ATTR_PRINTF(1, 2)
_Noreturn void
irc_util_die(const char *, ...);

#endif

int
irc_util_stoi(const char *, long long *);

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
