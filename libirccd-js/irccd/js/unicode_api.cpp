/*
 * unicode_api.cpp -- Irccd.Unicode API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include "plugin.hpp"
#include "unicode.hpp"
#include "unicode_api.hpp"

using irccd::daemon::bot;

namespace irccd::js {

namespace {

// {{{ Irccd.Unicode.isDigit

/*
 * Function: Irccd.Unicode.isDigit(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is in the digit category.
 */
auto Unicode_isDigit(duk_context* ctx) noexcept -> duk_ret_t
{
	return duk::push(ctx, unicode::isdigit(duk_get_int(ctx, 0)));
}

// }}}

// {{{ Irccd.Unicode.isLetter

/*
 * Function: Irccd.Unicode.isLetter(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is in the letter category.
 */
auto Unicode_isLetter(duk_context* ctx) noexcept -> duk_ret_t
{
	return duk::push(ctx, unicode::isalpha(duk_get_int(ctx, 0)));
}

// }}}

// {{{ Irccd.Unicode.isLower

/*
 * Function: Irccd.Unicode.isLower(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is lower case.
 */
auto Unicode_isLower(duk_context* ctx) noexcept -> duk_ret_t
{
	return duk::push(ctx, unicode::islower(duk_get_int(ctx, 0)));
}

// }}}

// {{{ Irccd.Unicode.isSpace

/*
 * Function: Irccd.Unicode.isSpace(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is in the space category.
 */
auto Unicode_isSpace(duk_context* ctx) noexcept -> duk_ret_t
{
	return duk::push(ctx, unicode::isspace(duk_get_int(ctx, 0)));
}

// }}}

// {{{ Irccd.Unicode.isTitle

/*
 * Function: Irccd.Unicode.isTitle(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is title case.
 */
auto Unicode_isTitle(duk_context* ctx) noexcept -> duk_ret_t
{
	return duk::push(ctx, unicode::istitle(duk_get_int(ctx, 0)));
}

// }}}

// {{{ Irccd.Unicode.isUpper

/*
 * Function: Irccd.Unicode.isUpper(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is upper case.
 */
auto Unicode_isUpper(duk_context* ctx) noexcept -> duk_ret_t
{
	return duk::push(ctx, unicode::isupper(duk_get_int(ctx, 0)));
}

// }}}

const duk_function_list_entry functions[] = {
	{ "isDigit",            Unicode_isDigit,        1 },
	{ "isLetter",           Unicode_isLetter,       1 },
	{ "isLower",            Unicode_isLower,        1 },
	{ "isSpace",            Unicode_isSpace,        1 },
	{ "isTitle",            Unicode_isTitle,        1 },
	{ "isUpper",            Unicode_isUpper,        1 },
	{ nullptr,              nullptr,                0 }
};

} // !namespace

auto unicode_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Unicode";
}

void unicode_api::load(bot&, std::shared_ptr<plugin> plugin)
{
	duk::stack_guard sa(plugin->get_context());

	duk_get_global_string(plugin->get_context(), "Irccd");
	duk_push_object(plugin->get_context());
	duk_put_function_list(plugin->get_context(), -1, functions);
	duk_put_prop_string(plugin->get_context(), -2, "Unicode");
	duk_pop(plugin->get_context());
}

} // !irccd::js
