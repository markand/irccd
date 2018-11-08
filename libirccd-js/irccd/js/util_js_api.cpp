/*
 * util_js_api.cpp -- Irccd.Util API
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

#include <climits>

#include <irccd/string_util.hpp>

#include "irccd_js_api.hpp"
#include "js_plugin.hpp"
#include "util_js_api.hpp"

namespace irccd::js {

namespace {

// {{{ subst

/*
 * Read parameters for Irccd.Util.format function, the object is defined as
 * following:
 *
 * {
 *   date: the date object
 *   flags: the flags (not implemented yet)
 *   field1: a field to substitute in #{} pattern
 *   field2: a field to substitute in #{} pattern
 *   fieldn: ...
 * }
 */
auto subst(duk_context* ctx, int index) -> string_util::subst
{
	string_util::subst params;

	if (!duk_is_object(ctx, index))
		return params;

	duk_enum(ctx, index, 0);

	while (duk_next(ctx, -1, true)) {
		if (duk::get<std::string>(ctx, -2) == "date")
			params.time = static_cast<time_t>(duk_get_number(ctx, -1) / 1000);
		else
			params.keywords.insert({
				duk::get<std::string>(ctx, -2),
				duk::get<std::string>(ctx, -1)
			});

		duk_pop_n(ctx, 2);
	}

	return params;
}

// }}}

// {{{ split

/*
 * split (for Irccd.Util.cut)
 * ------------------------------------------------------------------
 *
 * Extract individual tokens in array or a whole string as a std:::vector.
 */
auto split(duk_context* ctx) -> std::vector<std::string>
{
	duk_require_type_mask(ctx, 0, DUK_TYPE_MASK_OBJECT | DUK_TYPE_MASK_STRING);

	std::vector<std::string> result;
	std::string pattern = " \t\n";

	if (duk_is_string(ctx, 0))
		result = string_util::split(duk::get<std::string>(ctx, 0), pattern);
	else if (duk_is_array(ctx, 0)) {
		duk_enum(ctx, 0, DUK_ENUM_ARRAY_INDICES_ONLY);

		while (duk_next(ctx, -1, 1)) {
			// Split individual tokens as array if spaces are found.
			const auto tmp = string_util::split(duk_to_string(ctx, -1), pattern);

			result.insert(result.end(), tmp.begin(), tmp.end());
			duk_pop_2(ctx);
		}
	}

	return result;
}

// }}}

// {{{ limit

/*
 * limit (for Irccd.Util.cut)
 * ------------------------------------------------------------------
 *
 * Get the maxl/maxc argument.
 *
 * The argument value is the default and also used as the result returned.
 */
auto limit(duk_context* ctx, int index, const char* name, int value) -> int
{
	if (duk_get_top(ctx) < index || !duk_is_number(ctx, index))
		return value;

	value = duk_to_int(ctx, index);

	if (value <= 0)
		duk_error(ctx, DUK_ERR_RANGE_ERROR, "argument %d (%s) must be positive", index, name);

	return value;
}

// }}}

// {{{ lines

/*
 * lines (for Irccd.Util.cut)
 * ------------------------------------------------------------------
 *
 * Build a list of lines.
 *
 * Several cases possible:
 *
 *   - s is the current line
 *   - abc is the token to add
 *
 * s   = ""				 (new line)
 * s  -> "abc"
 *
 * s   = "hello world"	  (enough room)
 * s  -> "hello world abc"
 *
 * s   = "hello world"	  (not enough room: maxc is smaller)
 * s+1 = "abc"
 */
auto lines(duk_context* ctx, const std::vector<std::string>& tokens, int maxc) -> std::vector<std::string>
{
	std::vector<std::string> result{""};

	for (const auto& s : tokens) {
		if (s.length() > static_cast<std::size_t>(maxc))
			duk_error(ctx, DUK_ERR_RANGE_ERROR, "word '%s' could not fit in maxc limit (%d)", s.c_str(), maxc);

		// Compute the length required (prepend a space if needed)
		auto required = s.length() + (result.back().empty() ? 0 : 1);

		if (result.back().length() + required > static_cast<std::size_t>(maxc))
			result.push_back(s);
		else {
			if (!result.back().empty())
				result.back() += ' ';

			result.back() += s;
		}
	}

	return result;
}

// }}}

// {{{ wrap

template <typename Handler>
auto wrap(duk_context* ctx, Handler handler) -> duk_ret_t
{
	try {
		return handler();
	} catch (const std::system_error& ex) {
		duk::raise(ctx, ex);
	} catch (const std::exception& ex) {
		duk::raise(ctx, ex);
	}

	return 0;
}

// }}}

// {{{ Irccd.Util.cut

/*
 * Function: Irccd.Util.cut(data, maxc, maxl)
 * --------------------------------------------------------
 *
 * Cut a piece of data into several lines.
 *
 * The argument data is a string or a list of strings. In any case, all strings
 * are first splitted by spaces and trimmed. This ensure that useless
 * whitespaces are discarded.
 *
 * The argument maxc controls the maximum of characters allowed per line, it can
 * be a positive integer. If undefined is given, a default of 72 is used.
 *
 * The argument maxl controls the maximum of lines allowed. It can be a positive
 * integer or undefined for an infinite list.
 *
 * If maxl is used as a limit and the data can not fit within the bounds,
 * undefined is returned.
 *
 * An empty list may be returned if empty strings were found.
 *
 * Arguments:
 *   - data, a string or an array of strings,
 *   - maxc, max number of colums (Optional, default: 72),
 *   - maxl, max number of lines (Optional, default: undefined).
 * Returns:
 *   A list of strings ready to be sent or undefined if the data is too big.
 * Throws:
 *   - RangeError if maxl or maxc are negative numbers,
 *   - RangeError if one word length was bigger than maxc,
 *   - TypeError if data is not a string or a list of strings,
 *   - Irccd.SystemError on other errors.
 */
auto Util_cut(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		const auto list = lines(ctx, split(ctx), limit(ctx, 1, "maxc", 72));
		const auto maxl = limit(ctx, 2, "maxl", INT_MAX);

		if (list.size() > static_cast<std::size_t>(maxl))
			return 0;

		// Empty list but lines() returns at least one.
		if (list.size() == 1 && list[0].empty()) {
			duk_push_array(ctx);
			return 1;
		}

		return duk::push(ctx, list);
	});
}

// }}}

// {{{ Irccd.Util.format

/*
 * Function: Irccd.Util.format(text, parameters)
 * --------------------------------------------------------
 *
 * Format a string with templates.
 *
 * Arguments:
 *   - input, the text to update,
 *   - params, the parameters.
 * Returns:
 *   The converted text.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Util_format(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return duk::push(ctx, string_util::format(duk::get<std::string>(ctx, 0), subst(ctx, 1)));
	});
}

// }}}

// {{{ Irccd.Util.splituser

/*
 * Function: Irccd.Util.splituser(ident)
 * --------------------------------------------------------
 *
 * Return the nickname part from a full username.
 *
 * Arguments:
 *   - ident, the full identity.
 * Returns:
 *   The nickname.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Util_splituser(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return duk::push(ctx, irc::user::parse(duk::require<std::string>(ctx, 0)).nick);
	});
}

// }}}

// {{{ Irccd.Util.splithost

/*
 * Function: Irccd.Util.splithost(ident)
 * --------------------------------------------------------
 *
 * Return the hostname part from a full username.
 *
 * Arguments:
 *   - ident, the full identity.
 * Returns:
 *   The hostname.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
auto Util_splithost(duk_context* ctx) -> duk_ret_t
{
	return wrap(ctx, [&] {
		return duk::push(ctx, irc::user::parse(duk::require<std::string>(ctx, 0)).host);
	});
}

// }}}

const duk_function_list_entry functions[] = {
	{ "cut",        Util_cut,       DUK_VARARGS     },
	{ "format",     Util_format,    DUK_VARARGS     },
	{ "splituser",  Util_splituser, 1               },
	{ "splithost",  Util_splithost, 1               },
	{ nullptr,      nullptr,        0               }
};

} // !namespace

auto util_js_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Util";
}

void util_js_api::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
	duk::stack_guard sa(plugin->get_context());

	duk_get_global_string(plugin->get_context(), "Irccd");
	duk_push_object(plugin->get_context());
	duk_put_function_list(plugin->get_context(), -1, functions);
	duk_put_prop_string(plugin->get_context(), -2, "Util");
	duk_pop(plugin->get_context());
}

} // !irccd::js