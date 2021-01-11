/*
 * dl-plugin.c -- native C plugins for irccd
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

#ifndef IRCCD_DL_PLUGIN_H
#define IRCCD_DL_PLUGIN_H

#include <stdbool.h>

struct irc_plugin;

#if defined(_WIN32)
#       define IRC_DL_EXPORT __declspec(dllexport)
#else
#       define IRC_DL_EXPORT
#endif

#if defined(__APPLE__)
#       define IRC_DL_EXT ".dylib"
#else
#       define IRC_DL_EXT ".so"
#endif

bool
irc_dl_plugin_open(struct irc_plugin *, const char *);

#endif /* !IRCCD_DL_PLUGIN_H */
