/*
 * set.h -- generic macros to insert/remove in sorted arrays
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

#ifndef IRCCD_SET_H
#define IRCCD_SET_H

#include <string.h>
#include <stdlib.h>

#include "util.h"

typedef int (*irc_set_cmp)(const void *, const void *);

#define IRC_SET_FIND(a, asz, o, f)                                              \
        bsearch(o, a, asz, sizeof (*(o)), (irc_set_cmp)f)

#define IRC_SET_ALLOC_PUSH(a, asz, o, f)                                        \
do {                                                                            \
        *(a) = irc_util_reallocarray(*(a), ++(*(asz)), sizeof (*(o)));          \
        memcpy(*(a) + ((*asz) - 1), o, sizeof (*o));                            \
        qsort(*(a), *(asz), sizeof (*o), (irc_set_cmp)f);                       \
} while (0)

#define IRC_SET_ALLOC_REMOVE(a, asz, o)                                         \
do {                                                                            \
        if (--(*(asz)) == 0) {                                                  \
                free(*(a));                                                     \
                *(a) = NULL;                                                    \
        } else {                                                                \
                memmove(o, o + 1, sizeof (*(o)) * (*(asz) - ((o) - *(a))));     \
                *(a) = irc_util_reallocarray(*(a), *(asz), sizeof (*(o)));      \
        }                                                                       \
} while (0)

#define IRC_SET_PUSH(a, asz, o, f)                                              \
do {                                                                            \
        memcpy(a + (*(asz))++, o, sizeof (*o));                                 \
        qsort(a, *(asz), sizeof (*o), (irc_set_cmp)f);                          \
} while (0)

#define IRC_SET_REMOVE(a, asz, o)                                               \
do {                                                                            \
        if (--(*asz) != 0)                                                      \
                memmove(o, o + 1, sizeof (*(o)) * (*(asz) - ((o) - (a))));      \
} while (0)

#endif /* !IRCCD_SET_H */
