/*
 * nce.h -- nano coroutine events
 *
 * Copyright (c) 2026 David Demelier <markand@malikania.fr>
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

#ifndef LIBNCE_H
#define LIBNCE_H

/**
 * \file nce/nce.h
 * \brief Miscellaneous helpers.
 */

#include <stddef.h>

#include <ev.h>

/**
 * Major version.
 */
#define NCE_VERSION_MAJOR 0

/**
 * Minor version.
 */
#define NCE_VERSION_MINOR 1

/**
 * Patch version.
 */
#define NCE_VERSION_PATCH 0

/**
 * Minimum priority.
 */
#define NCE_PRI_MIN EV_MINPRI

/**
 * Maximum priority.
 */
#define NCE_PRI_MAX EV_MAXPRI

/**
 * Macro to retrieve original structure pointer from a inner field.
 *
 * Example:
 *
 * ```c
 * struct driver {
 *     int foo;
 *     int bar;
 *     struct coro coro;
 * };
 *
 * static void
 * driver_entry(EV_P_ struct coro *self)
 * {
 *     struct driver *driver = NCE_CONTAINER_OF(self, struct driver, coro);
 * }
 * ```
 */
#if defined(DOXYGEN)
#define NCE_CONTAINER_OF(Ptr, Type, Member)
#else
#define NCE_CONTAINER_OF(Ptr, Type, Member) \
	((Type *)((char *)(1 ? (Ptr) : &((Type *)0)->Member) - offsetof(Type, Member)))
#endif

#endif /* !LIBNCE_H */
