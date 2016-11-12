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

#include "duktape.hpp"
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
duk_ret_t isDigit(duk_context *ctx)
{
    duk_push_boolean(ctx, unicode::isdigit(duk_get_int(ctx, 0)));

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
duk_ret_t isLetter(duk_context *ctx)
{
    duk_push_boolean(ctx, unicode::isalpha(duk_get_int(ctx, 0)));

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
duk_ret_t isLower(duk_context *ctx)
{
    duk_push_boolean(ctx, unicode::islower(duk_get_int(ctx, 0)));

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
duk_ret_t isSpace(duk_context *ctx)
{
    duk_push_boolean(ctx, unicode::isspace(duk_get_int(ctx, 0)));

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
duk_ret_t isTitle(duk_context *ctx)
{
    duk_push_boolean(ctx, unicode::istitle(duk_get_int(ctx, 0)));

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
duk_ret_t isUpper(duk_context *ctx)
{
    duk_push_boolean(ctx, unicode::isupper(duk_get_int(ctx, 0)));

    return 1;
}

const duk_function_list_entry functions[] = {
    { "isDigit",        isDigit,    1 },
    { "isLetter",       isLetter,   1 },
    { "isLower",        isLower,    1 },
    { "isSpace",        isSpace,    1 },
    { "isTitle",        isTitle,    1 },
    { "isUpper",        isUpper,    1 },
    { nullptr,          nullptr,    0 }
};

} // !namespace

UnicodeModule::UnicodeModule() noexcept
    : Module("Irccd.Unicode")
{
}

void UnicodeModule::load(Irccd &, JsPlugin &plugin)
{
    StackAssert sa(plugin.context());

    duk_get_global_string(plugin.context(), "Irccd");
    duk_push_object(plugin.context());
    duk_put_function_list(plugin.context(), -1, functions);
    duk_put_prop_string(plugin.context(), -2, "Unicode");
    duk_pop(plugin.context());
}

} // !irccd
