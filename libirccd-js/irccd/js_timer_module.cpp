/*
 * js_timer_module.cpp -- Irccd.timer API
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/js_plugin.hpp>
#include <irccd/string_util.hpp>
#include <irccd/timer.hpp>

#include "js_irccd_module.hpp"
#include "js_plugin_module.hpp"
#include "js_timer_module.hpp"

namespace irccd {

namespace {

const char* signature("\xff""\xff""irccd-timer-ptr");
const char* callback_table("\xff""\xff""irccd-timer-callbacks");

void handle_signal(irccd& instance, std::weak_ptr<js_plugin> ptr, std::string key)
{
    auto plugin = ptr.lock();

    if (!plugin)
        return;

    instance.post([plugin, key] (irccd &) {
        StackAssert sa(plugin->context());

        duk_get_global_string(plugin->context(), callback_table);
        duk_get_prop_string(plugin->context(), -1, key.c_str());
        duk_remove(plugin->context(), -2);

        if (duk_is_callable(plugin->context(), -1)) {
            if (duk_pcall(plugin->context(), 0) != 0)
                log::warning(string_util::sprintf("plugin %s: %s", plugin->name(),
                    dukx_exception(plugin->context(), -1).stack));
            else
                duk_pop(plugin->context());
        } else
            duk_pop(plugin->context());
    });
}

std::shared_ptr<timer> self(duk_context* ctx)
{
    StackAssert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, signature);
    auto ptr = duk_to_pointer(ctx, -1);
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a timer object");

    return *static_cast<std::shared_ptr<timer>*>(ptr);
}

/*
 * Method: timer.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method is a no-op.
 */
duk_ret_t start(duk_context* ctx)
{
    auto timer = self(ctx);

    if (!timer->is_running())
        timer->start();

    return 0;
}

/*
 * Method: timer.stop()
 * --------------------------------------------------------
 *
 * Stop the timer.
 */
duk_ret_t stop(duk_context* ctx)
{
    auto timer = self(ctx);

    if (timer->is_running())
        timer->stop();

    return 0;
}

const duk_function_list_entry methods[] = {
    { "start",  start,      0 },
    { "stop",   stop,       0 },
    { nullptr,  nullptr,    0 }
};

/*
 * Function: Irccd.timer(type, delay, callback) [constructor]
 * --------------------------------------------------------
 *
 * Create a new timer object.
 *
 * Arguments:
 *   - type, the type of timer (irccd.timer.Single or irccd.timer.Repeat),
 *   - delay, the interval in milliseconds,
 *   - callback, the function to call.
 */
duk_ret_t constructor(duk_context* ctx)
{
    // Check parameters.
    auto type = duk_require_int(ctx, 0);
    auto delay = duk_require_int(ctx, 1);

    if (type < static_cast<int>(timer::type::single) || type > static_cast<int>(timer::type::repeat))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
    if (delay < 0)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "negative delay given");
    if (!duk_is_callable(ctx, 2))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

    // Construct the timer in 'this'.
    auto& irccd = dukx_get_irccd(ctx);
    auto tm = std::make_shared<timer>(static_cast<timer::type>(type), delay);
    auto hash = std::to_string(reinterpret_cast<std::uintptr_t>(tm.get()));

    tm->on_signal.connect(std::bind(handle_signal, std::ref(irccd),
        std::weak_ptr<js_plugin>(dukx_get_plugin(ctx)), hash));

    duk_push_this(ctx);
    duk_push_pointer(ctx, new std::shared_ptr<timer>(std::move(tm)));
    duk_put_prop_string(ctx, -2, signature);
    duk_push_string(ctx, hash.c_str());
    duk_put_prop_string(ctx, -2, "\xff""\xff""timer-key");
    duk_push_c_function(ctx, [] (duk_context* ctx) -> duk_ret_t {
        StackAssert sa(ctx);

        duk_get_prop_string(ctx, 0, "\xff""\xff""timer-key");
        auto hash = duk_get_string(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_string(ctx, 0, signature);
        static_cast<std::shared_ptr<timer>*>(duk_to_pointer(ctx, -1))->get()->stop();
        delete static_cast<std::shared_ptr<timer>*>(duk_to_pointer(ctx, -1));
        duk_pop(ctx);
        duk_get_global_string(ctx, callback_table);
        duk_del_prop_string(ctx, -1, hash);
        duk_pop(ctx);
        log::debug("plugin: timer destroyed");

        return 0;
    }, 1);
    duk_set_finalizer(ctx, -2);

    // Save a callback function into the callback table.
    duk_get_global_string(ctx, callback_table);
    duk_dup(ctx, 2);
    duk_put_prop_string(ctx, -2, hash.c_str());
    duk_pop(ctx);

    return 0;
}

const duk_number_list_entry constants[] = {
    { "Single",     static_cast<int>(timer::type::single)   },
    { "Repeat",     static_cast<int>(timer::type::repeat)   },
    { nullptr,      0                                       }
};

} // !namespace

js_timer_module::js_timer_module() noexcept
    : module("Irccd.timer")
{
}

void js_timer_module::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    StackAssert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_c_function(plugin->context(), constructor, 3);
    duk_put_number_list(plugin->context(), -1, constants);
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, methods);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "Timer");
    duk_pop(plugin->context());
    duk_push_object(plugin->context());
    duk_put_global_string(plugin->context(), callback_table);
}

} // !irccd
