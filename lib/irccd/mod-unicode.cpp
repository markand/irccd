/*
 * js-unicode.cpp -- Irccd.Unicode API
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include "js.hpp"
#include "mod-unicode.hpp"
#include "plugin-js.hpp"
#include "unicode.hpp"

namespace irccd {

namespace {

/*
 * Function: Irccd.Unicode.isDigit(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is in the digit category.
 */
duk::Ret isDigit(duk::Context *ctx)
{
	duk::push(ctx, unicode::isdigit(duk::get<int>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isLetter(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is in the letter category.
 */
duk::Ret isLetter(duk::Context *ctx)
{
	duk::push(ctx, unicode::isalpha(duk::get<int>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isLower(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is lower case.
 */
duk::Ret isLower(duk::Context *ctx)
{
	duk::push(ctx, unicode::islower(duk::get<int>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isSpace(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is in the space category.
 */
duk::Ret isSpace(duk::Context *ctx)
{
	duk::push(ctx, unicode::isspace(duk::get<int>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isTitle(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is title case.
 */
duk::Ret isTitle(duk::Context *ctx)
{
	duk::push(ctx, unicode::istitle(duk::get<int>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isUpper(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point.
 * Returns:
 *   True if the code is upper case.
 */
duk::Ret isUpper(duk::Context *ctx)
{
	duk::push(ctx, unicode::isupper(duk::get<int>(ctx, 0)));

	return 1;
}

const duk::FunctionMap functions{
	{ "isDigit",		{ isDigit,	1	} },
	{ "isLetter",		{ isLetter,	1	} },
	{ "isLower",		{ isLower,	1	} },
	{ "isSpace",		{ isSpace,	1	} },
	{ "isTitle",		{ isTitle,	1	} },
	{ "isUpper",		{ isUpper,	1	} },
};

} // !namespace

UnicodeModule::UnicodeModule() noexcept
	: Module("Irccd.Unicode")
{
}

void UnicodeModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Object{});
	duk::put(plugin.context(), functions);
	duk::putProperty(plugin.context(), -2, "Unicode");
	duk::pop(plugin.context());
}

} // !irccd
