/*
 * js-timer.cpp -- Irccd.Timer API
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

#include <format.h>

#include "irccd.hpp"
#include "logger.hpp"
#include "mod-irccd.hpp"
#include "mod-timer.hpp"
#include "mod-plugin.hpp"
#include "plugin-js.hpp"
#include "timer.hpp"

using namespace fmt::literals;

namespace irccd {

namespace {

const char *Signature("\xff""\xff""irccd-timer-ptr");
const char *CallbackTable("\xff""\xff""irccd-timer-callbacks");

void handleSignal(std::weak_ptr<JsPlugin> ptr, std::string key)
{
    auto plugin = ptr.lock();

    if (!plugin)
        return;

    auto &irccd = dukx_get_irccd(plugin->context());

    irccd.post([plugin, key] (Irccd &) {
        StackAssert sa(plugin->context());

        duk_get_global_string(plugin->context(), CallbackTable);
        duk_get_prop_string(plugin->context(), -1, key.c_str());
        duk_remove(plugin->context(), -2);

        if (duk_is_callable(plugin->context(), -1)) {
            if (duk_pcall(plugin->context(), 0) != 0)
                log::warning("plugin {}: {}"_format(plugin->name(), dukx_exception(plugin->context(), -1).stack));
            else
                duk_pop(plugin->context());
        } else
            duk_pop(plugin->context());
    });
}

std::shared_ptr<Timer> self(duk_context *ctx)
{
    StackAssert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, Signature);
    auto ptr = duk_to_pointer(ctx, -1);
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Timer object");

    return *static_cast<std::shared_ptr<Timer> *>(ptr);
}

/*
 * Method: Timer.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method is a no-op.
 */
duk_ret_t start(duk_context *ctx)
{
    auto timer = self(ctx);

    if (!timer->isRunning())
        timer->start();

    return 0;
}

/*
 * Method: Timer.stop()
 * --------------------------------------------------------
 *
 * Stop the timer.
 */
duk_ret_t stop(duk_context *ctx)
{
    auto timer = self(ctx);

    if (timer->isRunning())
        timer->stop();

    return 0;
}

const duk_function_list_entry methods[] = {
    { "start",  start,      0 },
    { "stop",   stop,       0 },
    { nullptr,  nullptr,    0 }
};

/*
 * Function: Irccd.Timer(type, delay, callback) [constructor]
 * --------------------------------------------------------
 *
 * Create a new timer object.
 *
 * Arguments:
 *   - type, the type of timer (Irccd.Timer.Single or Irccd.Timer.Repeat),
 *   - delay, the interval in milliseconds,
 *   - callback, the function to call.
 */
duk_ret_t constructor(duk_context *ctx)
{
    // Check parameters.
    auto type = duk_require_int(ctx, 0);
    auto delay = duk_require_int(ctx, 1);

    if (type < static_cast<int>(TimerType::Single) || type > static_cast<int>(TimerType::Repeat))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
    if (delay < 0)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "negative delay given");
    if (!duk_is_callable(ctx, 2))
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

    // Construct the timer in 'this'.
    auto timer = std::make_shared<Timer>(static_cast<TimerType>(type), delay);
    auto hash = std::to_string(reinterpret_cast<std::uintptr_t>(timer.get()));

    timer->onSignal.connect(std::bind(handleSignal, std::weak_ptr<JsPlugin>(dukx_get_plugin(ctx)), hash));

    duk_push_this(ctx);
    duk_push_pointer(ctx, new std::shared_ptr<Timer>(std::move(timer)));
    duk_put_prop_string(ctx, -2, Signature);
    duk_push_string(ctx, hash.c_str());
    duk_put_prop_string(ctx, -2, "\xff""\xff""timer-key");
    duk_push_c_function(ctx, [] (duk_context *ctx) -> duk_ret_t {
        StackAssert sa(ctx);

        duk_get_prop_string(ctx, 0, "\xff""\xff""timer-key");
        auto hash = duk_get_string(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_string(ctx, 0, Signature);
        static_cast<std::shared_ptr<Timer> *>(duk_to_pointer(ctx, -1))->get()->stop();
        delete static_cast<std::shared_ptr<Timer> *>(duk_to_pointer(ctx, -1));
        duk_pop(ctx);
        duk_get_global_string(ctx, CallbackTable);
        duk_del_prop_string(ctx, -1, hash);
        duk_pop(ctx);
        log::debug("plugin: timer destroyed");

        return 0;
    }, 1);
    duk_set_finalizer(ctx, -2);

    // Save a callback function into the callback table.
    duk_get_global_string(ctx, CallbackTable);
    duk_dup(ctx, 2);
    duk_put_prop_string(ctx, -2, hash.c_str());
    duk_pop(ctx);

    return 0;
}

const duk_number_list_entry constants[] = {
    { "Single",     static_cast<int>(TimerType::Single) },
    { "Repeat",     static_cast<int>(TimerType::Repeat) },
    { nullptr,      0                                   }
};

} // !namespace

TimerModule::TimerModule() noexcept
    : Module("Irccd.Timer")
{
}

void TimerModule::load(Irccd &, std::shared_ptr<JsPlugin> plugin)
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
    duk_put_global_string(plugin->context(), CallbackTable);
}

} // !irccd
