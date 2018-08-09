/*
 * logger_js_api.cpp -- Irccd.Logger API
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

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include "irccd_js_api.hpp"
#include "js_plugin.hpp"
#include "logger_js_api.hpp"
#include "plugin_js_api.hpp"

namespace irccd::js {

namespace {

// {{{ print

auto print(duk_context* ctx, unsigned level) -> duk_ret_t
{
    assert(level <= 2);

    try {
        auto& sink = duk::type_traits<irccd>::self(ctx).get_log();
        auto& self = duk::type_traits<js_plugin>::self(ctx);

        switch (level) {
        case 0:
            sink.debug<plugin>(self) << duk_require_string(ctx, 0) << std::endl;
            break;
        case 1:
            sink.info<plugin>(self) << duk_require_string(ctx, 0) << std::endl;
            break;
        default:
            sink.warning<plugin>(self) << duk_require_string(ctx, 0) << std::endl;
            break;
        }
    } catch (const std::exception& ex) {
        duk::raise(ctx, ex);
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
auto Logger_info(duk_context* ctx) -> duk_ret_t
{
    return print(ctx, 1);
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
auto Logger_warning(duk_context* ctx) -> duk_ret_t
{
    return print(ctx, 2);
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
auto Logger_debug(duk_context* ctx) -> duk_ret_t
{
    return print(ctx, 0);
}

// }}}

const duk_function_list_entry functions[] = {
    { "info",       Logger_info,    1 },
    { "warning",    Logger_warning, 1 },
    { "debug",      Logger_debug,   1 },
    { nullptr,      nullptr,        0 }
};

} // !namespace

auto logger_js_api::get_name() const noexcept -> std::string_view
{
    return "Irccd.Logger";
}

void logger_js_api::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    duk::stack_guard sa(plugin->get_context());

    duk_get_global_string(plugin->get_context(), "Irccd");
    duk_push_object(plugin->get_context());
    duk_put_function_list(plugin->get_context(), -1, functions);
    duk_put_prop_string(plugin->get_context(), -2, "Logger");
    duk_pop(plugin->get_context());
}

} // !irccd::js
