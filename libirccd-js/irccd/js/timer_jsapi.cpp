/*
 * timer_jsapi.cpp -- Irccd.timer API
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

#include <boost/asio.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"
#include "plugin_jsapi.hpp"
#include "timer_jsapi.hpp"

namespace irccd {

namespace {

const char* signature("\xff""\xff""irccd-timer-ptr");
const char* table("\xff""\xff""irccd-timer-callbacks");

// {{{ timer

class timer {
public:
    enum class type_t {
        single,
        repeat
    };

private:
    boost::asio::deadline_timer handle_;
    std::weak_ptr<js_plugin> plugin_;
    std::uintmax_t delay_;
    type_t type_;
    bool is_running_{false};
    bool is_waiting_{false};

    void handle();

public:
    inline timer(boost::asio::io_service& service,
                 std::weak_ptr<js_plugin> plugin,
                 std::uintmax_t delay,
                 type_t type) noexcept
        : handle_(service)
        , plugin_(plugin)
        , delay_(delay)
        , type_(type)
    {
    }

    inline std::string key() const
    {
        return std::to_string(reinterpret_cast<std::uintptr_t>(this));
    }

    void start();
    void stop();
};

void timer::handle()
{
    auto plg = plugin_.lock();

    if (!plg)
        return;

    auto& ctx = plg->get_context();

    duk_get_global_string(ctx, table);
    duk_get_prop_string(ctx, -1, key().c_str());
    duk_remove(ctx, -2);

    if (duk_pcall(ctx, 0)) {
        auto& log = dukx_type_traits<irccd>::self(ctx).get_log();

        log.warning(static_cast<const plugin&>(*plg)) << "timer error:" << std::endl;
        log.warning(static_cast<const plugin&>(*plg)) << "  " << dukx_stack(ctx, -1).what() << std::endl;
    } else
        duk_pop(ctx);
}

void timer::start()
{
    if (is_waiting_)
        return;

    is_running_ = is_waiting_ = true;

    handle_.expires_from_now(boost::posix_time::milliseconds(delay_));
    handle_.async_wait([this] (auto code) {
        is_waiting_ = false;

        if (code)
            return;

        handle();

        if (is_running_ && type_ == type_t::repeat)
            start();
    });
}

void timer::stop()
{
    if (is_running_) {
        handle_.cancel();
        is_running_ = false;
    }
}

// }}}

// {{{ self

timer* self(duk_context* ctx)
{
    dukx_stack_assert sa(ctx);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, signature);
    auto ptr = duk_to_pointer(ctx, -1);
    duk_pop_2(ctx);

    if (!ptr)
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Timer object");

    return static_cast<timer*>(ptr);
}

// }}}

// {{{ Irccd.Timer.prototype.start

/*
 * Method: Irccd.Timer.prototype.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method is a no-op.
 */
duk_ret_t Timer_prototype_start(duk_context* ctx)
{
    self(ctx)->start();

    return 0;
}

// }}}

// {{{ Irccd.Timer.prototype.stop

/*
 * Method: Irccd.Timer.prototype.stop()
 * --------------------------------------------------------
 *
 * Stop the timer.
 */
duk_ret_t Timer_prototype_stop(duk_context* ctx)
{
    self(ctx)->stop();

    return 0;
}

// }}}

// {{{ Irccd.Timer [destructor]

/*
 * Function: Irccd.Timer() [destructor]
 * ------------------------------------------------------------------
 *
 * Deleter the timer.
 */
duk_ret_t Timer_destructor(duk_context* ctx)
{
    dukx_stack_assert sa(ctx);

    // Get timer from this.
    duk_get_prop_string(ctx, 0, signature);
    auto ptr = static_cast<timer*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    // Remove callback from timer table.
    duk_get_global_string(ctx, table);
    duk_del_prop_string(ctx, -1, ptr->key().c_str());
    duk_pop(ctx);

    delete ptr;

    return 0;
}

// }}}

// {{{ Irccd.Timer [constructor]

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
duk_ret_t Timer_constructor(duk_context* ctx)
{
    if (!duk_is_constructor_call(ctx))
        return 0;

    try {
        // Check parameters.
        const auto type = duk_require_int(ctx, 0);
        const auto delay = duk_require_int(ctx, 1);

        if (type < static_cast<int>(timer::type_t::single) || type > static_cast<int>(timer::type_t::repeat))
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
        if (delay < 0)
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "negative delay given");
        if (!duk_is_callable(ctx, 2))
            duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

        auto plugin = dukx_type_traits<js_plugin>::self(ctx);
        auto& daemon = dukx_type_traits<irccd>::self(ctx);
        auto object = new timer(daemon.get_service(), plugin, delay, static_cast<timer::type_t>(type));

        duk_push_this(ctx);
        duk_push_pointer(ctx, object);
        duk_put_prop_string(ctx, -2, signature);
        duk_push_c_function(ctx, Timer_destructor, 1);
        duk_set_finalizer(ctx, -2);
        duk_pop(ctx);

        // Store the function in a table to be called later.
        duk_get_global_string(ctx, table);
        duk_dup(ctx, 2);
        duk_put_prop_string(ctx, -2, object->key().c_str());
    } catch (const std::exception& ex) {
        dukx_throw(ctx, ex);
    }

    return 0;
}

// }}}

const duk_function_list_entry methods[] = {
    { "start",  Timer_prototype_start,  0                   },
    { "stop",   Timer_prototype_stop,   0                   },
    { nullptr,  nullptr,                0                   }
};

const duk_number_list_entry constants[] = {
    { "Single",     static_cast<int>(timer::type_t::single) },
    { "Repeat",     static_cast<int>(timer::type_t::repeat) },
    { nullptr,      0                                       }
};

} // !namespace

std::string timer_jsapi::get_name() const
{
    return "Irccd.Timer";
}

void timer_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->get_context());

    duk_get_global_string(plugin->get_context(), "Irccd");
    duk_push_c_function(plugin->get_context(), Timer_constructor, 3);
    duk_put_number_list(plugin->get_context(), -1, constants);
    duk_push_object(plugin->get_context());
    duk_put_function_list(plugin->get_context(), -1, methods);
    duk_put_prop_string(plugin->get_context(), -2, "prototype");
    duk_put_prop_string(plugin->get_context(), -2, "Timer");
    duk_pop(plugin->get_context());
    duk_push_object(plugin->get_context());
    duk_put_global_string(plugin->get_context(), table);
}

} // !irccd
