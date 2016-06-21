/*
 * js-util.cpp -- Irccd.Util API
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

#include <libircclient.h>

#include "mod-util.hpp"
#include "plugin-js.hpp"
#include "util.hpp"

namespace irccd {

namespace {

/**
 * Read parameters for Irccd.Util.format function, the object is defined as follow:
 *
 * {
 *   date: the date object
 *   flags: the flags (not implemented yet)
 *   field1: a field to substitute in #{} pattern
 *   field2: a field to substitute in #{} pattern
 *   fieldn: ...
 * }
 */
util::Substitution getSubstitution(duk_context *ctx, int index)
{
    util::Substitution params;

    if (!duk_is_object(ctx, index))
        return params;

    dukx_enumerate(ctx, index, 0, true, [&] (duk_context *) {
        if (dukx_get_std_string(ctx, -2) == "date")
            params.time = static_cast<time_t>(duk_get_number(ctx, -1) / 1000);
        else
            params.keywords.insert({dukx_get_std_string(ctx, -2), dukx_get_std_string(ctx, -1)});
    });

    return params;
}

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
 */
duk_ret_t format(duk_context *ctx)
{
    try {
        dukx_push_std_string(ctx, util::format(dukx_get_std_string(ctx, 0), getSubstitution(ctx, 1)));
    } catch (const std::exception &ex) {
        dukx_throw(ctx, SyntaxError(ex.what()));
    }

    return 1;
}

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
 */
duk_ret_t splituser(duk_context *ctx)
{
    auto target = duk_require_string(ctx, 0);
    char nick[32] = {0};

    irc_target_get_nick(target, nick, sizeof (nick) -1);
    duk_push_string(ctx, nick);

    return 1;
}

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
 */
duk_ret_t splithost(duk_context *ctx)
{
    auto target = duk_require_string(ctx, 0);
    char host[32] = {0};

    irc_target_get_host(target, host, sizeof (host) -1);
    duk_push_string(ctx, host);

    return 1;
}

const duk_function_list_entry functions[] = {
    { "format",     format,     DUK_VARARGS },
    { "splituser",  splituser,  1           },
    { "splithost",  splithost,  1           },
    { nullptr,      nullptr,    0           }
};

} // !namespace

UtilModule::UtilModule() noexcept
    : Module("Irccd.Util")
{
}

void UtilModule::load(Irccd &, const std::shared_ptr<JsPlugin> &plugin)
{
    StackAssert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, functions);
    duk_put_prop_string(plugin->context(), -2, "Util");
    duk_pop(plugin->context());
}

} // !irccd
