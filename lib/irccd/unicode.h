/*
 * unicode.h -- UTF-8 to UTF-32 conversions and various operations
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_UNICODE_H
#define IRCCD_UNICODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t
irc_uni8_encode(uint8_t dst[], size_t dstsz, uint32_t point);

size_t
irc_uni8_decode(const uint8_t src[], uint32_t *point);

size_t
irc_uni8_sizeof(uint8_t c);

size_t
irc_uni8_length(const uint8_t src[]);

size_t
irc_uni8_to32(const uint8_t src[], uint32_t dst[], size_t dstsz);

size_t
irc_uni32_sizeof(uint32_t point);

size_t
irc_uni32_length(const uint32_t src[]);

size_t
irc_uni32_requires(const uint32_t src[]);

size_t
irc_uni32_to8(const uint32_t src[], uint8_t dst[], size_t dstsz);

bool
irc_uni_isalpha(uint32_t c);

bool
irc_uni_isdigit(uint32_t c);

bool
irc_uni_islower(uint32_t c);

bool
irc_uni_isspace(uint32_t c);

bool
irc_uni_istitle(uint32_t c);

bool
irc_uni_isupper(uint32_t c);

uint32_t
irc_uni_toupper(uint32_t c);

uint32_t
irc_uni_tolower(uint32_t c);

uint32_t
irc_uni_totitle(uint32_t c);

#endif /* !IRCCD_UNICODE_H */
