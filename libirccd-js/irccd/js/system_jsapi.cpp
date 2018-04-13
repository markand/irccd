/*
 * system_jsapi.cpp -- Irccd.System API
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

#include <irccd/sysconfig.hpp>

#include <chrono>
#include <cstdlib>
#include <thread>

#if defined(HAVE_POPEN)
#  include <cstdio>
#endif

#include <irccd/system.hpp>

#include "file_jsapi.hpp"
#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"
#include "system_jsapi.hpp"

namespace irccd {

namespace {

// {{{ wrap

template <typename Handler>
duk_ret_t wrap(duk_context* ctx, Handler handler)
{
    try {
        return handler();
    } catch (const std::system_error& ex) {
        dukx_throw(ctx, ex);
    } catch (const std::exception& ex) {
        dukx_throw(ctx, ex);
    }

    return 0;
}

// }}}

// {{{ Irccd.System.env

/*
 * Function: Irccd.System.env(key)
 * ------------------------------------------------------------------
 *
 * Get an environment system variable.
 *
 * Arguments:
 *   - key, the environment variable.
 * Returns:
 *   The value.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_env(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, sys::env(dukx_get<std::string>(ctx, 0)));
    });
}

// }}}

// {{{ Irccd.System.exec

/*
 * Function: Irccd.System.exec(cmd)
 * ------------------------------------------------------------------
 *
 * Execute a system command.
 *
 * Arguments:
 *   - cmd, the command to execute.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_exec(duk_context* ctx)
{
    std::system(duk_require_string(ctx, 0));

    return 0;
}

// }}}

// {{{ Irccd.System.home

/*
 * Function: Irccd.System.home()
 * ------------------------------------------------------------------
 *
 * Get the operating system user's home.
 *
 * Returns:
 *   The user home directory.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_home(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, sys::home());
    });
}

// }}}

// {{{ Irccd.System.name

/*
 * Function: Irccd.System.name()
 * ------------------------------------------------------------------
 *
 * Get the operating system name.
 *
 * Returns:
 *   The system name.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_name(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, sys::name());
    });
}

// }}}

// {{{ Irccd.System.popen

#if defined(HAVE_POPEN)

/*
 * Function: Irccd.System.popen(cmd, mode) [optional]
 * ------------------------------------------------------------------
 *
 * Wrapper for popen(3) if the function is available.
 *
 * Arguments:
 *   - cmd, the command to execute,
 *   - mode, the mode (e.g. "r").
 * Returns:
 *   A irccd.File object.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_popen(duk_context* ctx)
{
    return wrap(ctx, [&] {
        auto fp = ::popen(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

        if (fp == nullptr)
            throw std::system_error(make_error_code(static_cast<std::errc>(errno)));

        return dukx_push(ctx, std::make_shared<file>(fp, [] (auto fp) { ::pclose(fp); }));
    });
}

#endif // !HAVE_POPEN

// }}}

// {{{ Icrcd.System.sleep

/*
 * Function: Irccd.System.sleep(delay)
 * ------------------------------------------------------------------
 *
 * Sleep the main loop for the specific delay in seconds.
 *
 * Arguments:
 *   - delay, the delay in seconds.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_sleep(duk_context* ctx)
{
    return wrap(ctx, [&] {
        std::this_thread::sleep_for(std::chrono::seconds(duk_get_int(ctx, 0)));

        return 0;
    });
}

// }}}

// {{{ Irccd.System.ticks

/*
 * Function: Irccd.System.ticks()
 * ------------------------------------------------------------------
 *
 * Get the number of milliseconds since irccd was started.
 *
 * Returns:
 *   The number of milliseconds.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_ticks(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push<unsigned>(ctx, sys::ticks());
    });
}

// }}}

// {{{ Irccd.System.usleep

/*
 * Function: Irccd.System.usleep(delay)
 * ------------------------------------------------------------------
 *
 * Sleep the main loop for the specific delay in microseconds.
 *
 * Arguments:
 *   - delay, the delay in microseconds.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_usleep(duk_context* ctx)
{
    return wrap(ctx, [&] {
        std::this_thread::sleep_for(std::chrono::microseconds(duk_get_int(ctx, 0)));

        return 0;
    });
}

// }}}

// {{{ Irccd.System.uptime

/*
 * Function: Irccd.System.uptime()
 * ------------------------------------------------------------------
 *
 * Get the system uptime.
 *
 * Returns:
 *   The system uptime.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_uptime(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push<unsigned>(ctx, sys::uptime());
    });
}

// }}}

// {{{ Irccd.System.version

/*
 * Function: Irccd.System.version()
 * ------------------------------------------------------------------
 *
 * Get the operating system version.
 *
 * Returns:
 *   The system version.
 * Throws:
 *   - Irccd.SystemError on errors.
 */
duk_ret_t System_version(duk_context* ctx)
{
    return wrap(ctx, [&] {
        return dukx_push(ctx, sys::version());
    });
}

// }}}

const duk_function_list_entry functions[] = {
    { "env",        System_env,     1 },
    { "exec",       System_exec,    1 },
    { "home",       System_home,    0 },
    { "name",       System_name,    0 },
#if defined(HAVE_POPEN)
    { "popen",      System_popen,   2 },
#endif
    { "sleep",      System_sleep,   1 },
    { "ticks",      System_ticks,   0 },
    { "uptime",     System_uptime,  0 },
    { "usleep",     System_usleep,  1 },
    { "version",    System_version, 0 },
    { nullptr,      nullptr,        0 }
};

} // !namespace

std::string system_jsapi::get_name() const
{
    return "Irccd.System";
}

void system_jsapi::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    dukx_stack_assert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, functions);
    duk_put_prop_string(plugin->context(), -2, "System");
    duk_pop(plugin->context());
}

} // !irccd
