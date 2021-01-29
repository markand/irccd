/*
 * js-plugin.h -- Javascript plugins
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

#ifndef IRCCD_JS_PLUGIN_H
#define IRCCD_JS_PLUGIN_H

#include <duktape.h>

#include "plugin.h"

struct irc_js_plugin_data {
	struct irc_plugin plugin;
	duk_context *ctx;
	char **options;
	char **templates;
	char **paths;
	char *license;
	char *version;
	char *author;
	char *description;
};

struct irc_plugin *
irc_js_plugin_open(const char *);

struct irc_plugin_loader *
irc_js_plugin_loader_new(void);

#endif /* !IRCCD_JS_PLUGIN_H */
