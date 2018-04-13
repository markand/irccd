/*
 * logger_jsapi.cpp -- Irccd.Logger API
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

#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/irccd.hpp>

#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"
#include "logger_jsapi.hpp"
#include "plugin_jsapi.hpp"

namespace irccd {

namespace {

// {{{ print

duk_ret_t print(duk_context* ctx, std::ostream &out)
{
    try {
        out << "plugin " << dukx_type_traits<js_plugin>::self(ctx)->get_name() << ": ";
        out << duk_require_string(ctx, 0) << std::endl;
    } catch (const std::exception& ex) {
        dukx_throw(ctx, ex);
    }

    return 0;
}

// }}}

// {{{ Irccd.Logger.info

/*
 * Function: Irccd.Logger.info(message)
 * --------------------------------------------------------
 *
 * Write a verbose message.
 *
 * Arguments:
 *   - message, the message.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t Logger_info(duk_context* ctx)
{
    return print(ctx, dukx_type_traits<irccd>::self(ctx).get_log().info());
}

// }}}

// {{{ Irccd.Logger.warning

/*
 * Function: Irccd.Logger.warning(message)
 * --------------------------------------------------------
 *
 * Write a warning message.
 *
 * Arguments:
 *   - message, the warning.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t Logger_warning(duk_context* ctx)
{
    return print(ctx, dukx_type_traits<irccd>::self(ctx).get_log().warning());
}

// }}}

// {{{ Irccd.Logger.debug

/*
 * Function: Irccd.Logger.debug(message)
 * --------------------------------------------------------
 *
 * Write a debug message, only shown if irccd is compiled in debug.
 *
 * Arguments:
 *   - message, the message.
 * Throws:
 *   - Irccd.SystemError on errors
 */
duk_ret_t Logger_debug(duk_context* ctx)
{
    return print(ctx, dukx_type_traits<irccd>::self(ctx).get_log().debug());
}

// }}}

const duk_function_list_entry functions[] = {
    { "info",       Logger_info,    1 },
    { "warning",    Logger_warning, 1 },
    { "debug",      Logger_debug,   1 },
    { nullptr,      nullptr,        0 }
};

} // !namespace

std::string logger_jsapi::get_name() const
{
    return "Irccd.Logger";
}

void logger_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, functions);
    duk_put_prop_string(plugin->context(), -2, "Logger");
    duk_pop(plugin->context());
}

} // !irccd
