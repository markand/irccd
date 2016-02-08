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

#include <unicode.h>

#include "js.h"

namespace irccd {

namespace {

/*
 * Function: Irccd.Unicode.isDigit(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point
 * Returns:
 *   - true if the code is in the digit category
 */
int isDigit(js::Context &ctx)
{
	ctx.push(unicode::isdigit(ctx.get<int>(0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isLetter(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point
 * Returns:
 *   - true if the code is in the letter category
 */
int isLetter(js::Context &ctx)
{
	ctx.push(unicode::isalpha(ctx.get<int>(0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isLower(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point
 * Returns:
 *   - true if the code is lower case
 */
int isLower(js::Context &ctx)
{
	ctx.push(unicode::islower(ctx.get<int>(0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isSpace(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point
 * Returns:
 *   - true if the code is in the space category
 */
int isSpace(js::Context &ctx)
{
	ctx.push(unicode::isspace(ctx.get<int>(0)));

	return 1;
}

/*
 * Function: Unicode.isTitle(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point
 * Returns:
 *   - true if the code is title case
 */
int isTitle(js::Context &ctx)
{
	ctx.push(unicode::istitle(ctx.get<int>(0)));

	return 1;
}

/*
 * Function: Irccd.Unicode.isUpper(code)
 * --------------------------------------------------------
 *
 * Arguments:
 *   - code, the code point
 * Returns:
 *   - true if the code is upper case
 */
int isUpper(js::Context &ctx)
{
	ctx.push(unicode::isupper(ctx.get<int>(0)));

	return 1;
}

const js::FunctionMap functions{
	{ "isDigit",		{ isDigit,	1	} },
	{ "isLetter",		{ isLetter,	1	} },
	{ "isLower",		{ isLower,	1	} },
	{ "isSpace",		{ isSpace,	1	} },
	{ "isTitle",		{ isTitle,	1	} },
	{ "isUpper",		{ isUpper,	1	} },
};

} // !namespace

void loadJsUnicode(js::Context &ctx)
{
	ctx.getGlobal<void>("Irccd");
	ctx.push(js::Object{});
	ctx.push(functions);
	ctx.putProperty(-2, "Unicode");
	ctx.pop();
}

} // !irccd
