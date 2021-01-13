/*
 * jsapi-unicode.c -- Irccd.Unicode API
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

#include "jsapi-unicode.h"
#include "unicode.h"

static duk_ret_t
Unicode_isDigit(duk_context *ctx)
{
	duk_push_boolean(ctx, irc_uni_isdigit(duk_get_int(ctx, 0)));

	return 1;
}

static duk_ret_t
Unicode_isLetter(duk_context *ctx)
{
	duk_push_boolean(ctx, irc_uni_isalpha(duk_get_int(ctx, 0)));

	return 1;
}

static duk_ret_t
Unicode_isLower(duk_context *ctx)
{
	duk_push_boolean(ctx, irc_uni_islower(duk_get_int(ctx, 0)));

	return 1;
}

static duk_ret_t
Unicode_isSpace(duk_context *ctx)
{
	duk_push_boolean(ctx, irc_uni_isspace(duk_get_int(ctx, 0)));

	return 1;
}

static duk_ret_t
Unicode_isTitle(duk_context *ctx)
{
	duk_push_boolean(ctx, irc_uni_istitle(duk_get_int(ctx, 0)));

	return 1;
}

static duk_ret_t
Unicode_isUpper(duk_context *ctx)
{
	duk_push_boolean(ctx, irc_uni_isupper(duk_get_int(ctx, 0)));

	return 1;
}

static const duk_function_list_entry functions[] = {
	{ "isDigit",            Unicode_isDigit,        1 },
	{ "isLetter",           Unicode_isLetter,       1 },
	{ "isLower",            Unicode_isLower,        1 },
	{ "isSpace",            Unicode_isSpace,        1 },
	{ "isTitle",            Unicode_isTitle,        1 },
	{ "isUpper",            Unicode_isUpper,        1 },
	{ NULL,                 NULL,                   0 }
};

void
irc_jsapi_unicode_load(duk_context *ctx)
{
	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "Unicode");
	duk_pop(ctx);
}
