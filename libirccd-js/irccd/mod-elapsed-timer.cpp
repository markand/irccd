/*
 * mod-elapsed-timer.cpp -- Irccd.ElapsedTimer API
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

#include "elapsed-timer.hpp"
#include "mod-elapsed-timer.hpp"
#include "plugin-js.hpp"

namespace irccd {

namespace {

const char *Signature("\xff""\xff""irccd-elapsed-timer-ptr");

ElapsedTimer *self(duk_context *ctx)
{
    StackAssert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, Signature);
    auto ptr = static_cast<ElapsedTimer *>(duk_to_pointer(ctx, -1));
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not an ElapsedTimer object");

    return ptr;
}

/*
 * Method: ElapsedTimer.pause
 * ------------------------------------------------------------------
 *
 * Pause the timer, without resetting the current elapsed time stored.
 */
duk_ret_t pause(duk_context *ctx)
{
    self(ctx)->pause();

    return 0;
}

/*
 * Method: ElapsedTimer.reset
 * ------------------------------------------------------------------
 *
 * Reset the elapsed time to 0, the status is not modified.
 */
duk_ret_t reset(duk_context *ctx)
{
    self(ctx)->reset();

    return 0;
}

/*
 * Method: ElapsedTimer.restart
 * ------------------------------------------------------------------
 *
 * Restart the timer without resetting the current elapsed time.
 */
duk_ret_t restart(duk_context *ctx)
{
    self(ctx)->restart();

    return 0;
}

/*
 * Method: ElapsedTimer.elapsed
 * ------------------------------------------------------------------
 *
 * Get the number of elapsed milliseconds.
 *
 * Returns:
 *   The time elapsed.
 */
duk_ret_t elapsed(duk_context *ctx)
{
    duk_push_uint(ctx, self(ctx)->elapsed());

    return 1;
}

/*
 * Function: Irccd.ElapsedTimer() [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new ElapsedTimer object.
 */
duk_ret_t constructor(duk_context *ctx)
{
    duk_push_this(ctx);
    duk_push_pointer(ctx, new ElapsedTimer);
    duk_put_prop_string(ctx, -2, Signature);
    duk_pop(ctx);

    return 0;
}

/*
 * Function: Irccd.ElapsedTimer() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk_ret_t destructor(duk_context *ctx)
{
    duk_get_prop_string(ctx, 0, Signature);
    delete static_cast<ElapsedTimer *>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);
    duk_del_prop_string(ctx, 0, Signature);

    return 0;
}

const duk_function_list_entry methods[] = {
    { "elapsed",    elapsed,    0 },
    { "pause",      pause,      0 },
    { "reset",      reset,      0 },
    { "restart",    restart,    0 },
    { nullptr,      nullptr,    0 }
};

} // !namespace

ElapsedTimerModule::ElapsedTimerModule() noexcept
    : Module("Irccd.ElapsedTimer")
{
}

void ElapsedTimerModule::load(Irccd &, std::shared_ptr<JsPlugin> plugin)
{
    StackAssert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_c_function(plugin->context(), constructor, 0);
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, methods);
    duk_push_c_function(plugin->context(), destructor, 1);
    duk_set_finalizer(plugin->context(), -2);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "ElapsedTimer");
    duk_pop(plugin->context());
}

} // !irccd
