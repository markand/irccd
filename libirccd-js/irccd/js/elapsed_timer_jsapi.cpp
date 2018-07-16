/*
 * elapsed_timer_jsapi.cpp -- Irccd.ElapsedTimer API
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

#include <boost/timer/timer.hpp>

#include "elapsed_timer_jsapi.hpp"
#include "js_plugin.hpp"

namespace irccd {

namespace {

const char* signature("\xff""\xff""irccd-elapsed-timer-ptr");

// {{{ self

boost::timer::cpu_timer* self(duk_context* ctx)
{
    dukx_stack_assert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, signature);
    const auto ptr = static_cast<boost::timer::cpu_timer*>(duk_to_pointer(ctx, -1));
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not an ElapsedTimer object");

    return ptr;
}

// }}}

// {{{ Irccd.ElapsedTimer.prototype.pause

/*
 * Method: ElapsedTimer.prototype.pause
 * ------------------------------------------------------------------
 *
 * Pause the timer, without resetting the current elapsed time stored.
 */
duk_ret_t ElapsedTimer_prototype_pause(duk_context* ctx)
{
    self(ctx)->stop();

    return 0;
}

// }}}

// {{{ Irccd.ElapsedTimer.prototype.restart

/*
 * Method: Irccd.ElapsedTimer.prototype.restart
 * ------------------------------------------------------------------
 *
 * Restart the timer without resetting the current elapsed time.
 */
duk_ret_t ElapsedTimer_prototype_restart(duk_context* ctx)
{
    self(ctx)->resume();

    return 0;
}

// }}}

// {{{ Irccd.ElapsedTimer.prototype.elapsed

/*
 * Method: ElapsedTimer.prototype.elapsed
 * ------------------------------------------------------------------
 *
 * Get the number of elapsed milliseconds.
 *
 * Returns:
 *   The time elapsed.
 */
duk_ret_t ElapsedTimer_prototype_elapsed(duk_context* ctx)
{
    duk_push_uint(ctx, self(ctx)->elapsed().wall / 1000000LL);

    return 1;
}

// }}}

// {{{ Irccd.ElapsedTimer [constructor]

/*
 * Function: Irccd.ElapsedTimer [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new ElapsedTimer object.
 */
duk_ret_t ElapsedTimer_constructor(duk_context* ctx)
{
    duk_push_this(ctx);
    duk_push_pointer(ctx, new boost::timer::cpu_timer);
    duk_put_prop_string(ctx, -2, signature);
    duk_pop(ctx);

    return 0;
}

// }}}

// {{{ Irccd.ElapsedTimer [destructor]

/*
 * Function: Irccd.ElapsedTimer [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk_ret_t ElapsedTimer_destructor(duk_context* ctx)
{
    duk_get_prop_string(ctx, 0, signature);
    delete static_cast<boost::timer::cpu_timer*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);
    duk_del_prop_string(ctx, 0, signature);

    return 0;
}

// }}}

const duk_function_list_entry methods[] = {
    { "elapsed",    ElapsedTimer_prototype_elapsed, 0 },
    { "pause",      ElapsedTimer_prototype_pause,   0 },
    { "restart",    ElapsedTimer_prototype_restart, 0 },
    { nullptr,      nullptr,                        0 }
};

} // !namespace

std::string elapsed_timer_jsapi::get_name() const
{
    return "Irccd.ElapsedTimer";
}

void elapsed_timer_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->get_context());

    duk_get_global_string(plugin->get_context(), "Irccd");
    duk_push_c_function(plugin->get_context(), ElapsedTimer_constructor, 0);
    duk_push_object(plugin->get_context());
    duk_put_function_list(plugin->get_context(), -1, methods);
    duk_push_c_function(plugin->get_context(), ElapsedTimer_destructor, 1);
    duk_set_finalizer(plugin->get_context(), -2);
    duk_put_prop_string(plugin->get_context(), -2, "prototype");
    duk_put_prop_string(plugin->get_context(), -2, "ElapsedTimer");
    duk_pop(plugin->get_context());
}

} // !irccd
