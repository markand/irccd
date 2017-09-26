/*
 * js_system_module.cpp -- Irccd.System API
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

#include <chrono>
#include <cstdlib>
#include <thread>

#include <irccd/sysconfig.hpp>

#if defined(HAVE_POPEN)
#  include <cstdio>
#endif

#include <irccd/js_plugin.hpp>
#include <irccd/system.hpp>

#include "js_file_module.hpp"
#include "js_irccd_module.hpp"
#include "js_system_module.hpp"

namespace irccd {

namespace {

/*
 * Function: irccd.System.env(key)
 * ------------------------------------------------------------------
 *
 * Get an environment system variable.
 *
 * Arguments:
 *   - key, the environment variable.
 * Returns:
 *   The value.
 */
duk_ret_t env(duk_context* ctx)
{
    dukx_push_std_string(ctx, sys::env(dukx_get_std_string(ctx, 0)));

    return 1;
}

/*
 * Function: irccd.System.exec(cmd)
 * ------------------------------------------------------------------
 *
 * Execute a system command.
 *
 * Arguments:
 *   - cmd, the command to execute.
 */
duk_ret_t exec(duk_context* ctx)
{
    std::system(duk_get_string(ctx, 0));

    return 0;
}

/*
 * Function: irccd.System.home()
 * ------------------------------------------------------------------
 *
 * Get the operating system user's home.
 *
 * Returns:
 *   The user home directory.
 */
duk_ret_t home(duk_context* ctx)
{
    dukx_push_std_string(ctx, sys::home());

    return 1;
}

/*
 * Function: irccd.System.name()
 * ------------------------------------------------------------------
 *
 * Get the operating system name.
 *
 * Returns:
 *   The system name.
 */
duk_ret_t name(duk_context* ctx)
{
    dukx_push_std_string(ctx, sys::name());

    return 1;
}

#if defined(HAVE_POPEN)

/*
 * Function: irccd.System.popen(cmd, mode) [optional]
 * ------------------------------------------------------------------
 *
 * Wrapper for popen(3) if the function is available.
 *
 * Arguments:
 *   - cmd, the command to execute,
 *   - mode, the mode (e.g. "r").
 * Returns:
 *   A irccd.File object.
 * Throws
 *   - irccd.system_error on failures.
 */
duk_ret_t popen(duk_context* ctx)
{
    auto fp = ::popen(duk_require_string(ctx, 0), duk_require_string(ctx, 1));

    if (fp == nullptr)
        dukx_throw(ctx, system_error());

    dukx_push_file(ctx, new file(fp, [] (auto fp) { ::pclose(fp); }));

    return 1;
}

#endif // !HAVE_POPEN

/*
 * Function: irccd.System.sleep(delay)
 * ------------------------------------------------------------------
 *
 * Sleep the main loop for the specific delay in seconds.
 */
duk_ret_t sleep(duk_context* ctx)
{
    std::this_thread::sleep_for(std::chrono::seconds(duk_get_int(ctx, 0)));

    return 0;
}

/*
 * Function: irccd.System.ticks()
 * ------------------------------------------------------------------
 *
 * Get the number of milliseconds since irccd was started.
 *
 * Returns:
 *   The number of milliseconds.
 */
duk_ret_t ticks(duk_context* ctx)
{
    duk_push_int(ctx, sys::ticks());

    return 1;
}

/*
 * Function: irccd.System.usleep(delay)
 * ------------------------------------------------------------------
 *
 * Sleep the main loop for the specific delay in microseconds.
 */
duk_ret_t usleep(duk_context* ctx)
{
    std::this_thread::sleep_for(std::chrono::microseconds(duk_get_int(ctx, 0)));

    return 0;
}

/*
 * Function: irccd.System.uptime()
 * ------------------------------------------------------------------
 *
 * Get the system uptime.
 *
 * Returns:
 *   The system uptime.
 */
duk_ret_t uptime(duk_context* ctx)
{
    duk_push_int(ctx, sys::uptime());

    return 0;
}

/*
 * Function: irccd.System.version()
 * ------------------------------------------------------------------
 *
 * Get the operating system version.
 *
 * Returns:
 *   The system version.
 */
duk_ret_t version(duk_context* ctx)
{
    dukx_push_std_string(ctx, sys::version());

    return 1;
}

const duk_function_list_entry functions[] = {
    { "env",        env,        1 },
    { "exec",       exec,       1 },
    { "home",       home,       0 },
    { "name",       name,       0 },
#if defined(HAVE_POPEN)
    { "popen",      popen,      2 },
#endif
    { "sleep",      sleep,      1 },
    { "ticks",      ticks,      0 },
    { "uptime",     uptime,     0 },
    { "usleep",     usleep,     1 },
    { "version",    version,    0 },
    { nullptr,      nullptr,    0 }
};

} // !namespace

js_system_module::js_system_module() noexcept
    : module("Irccd.System")
{
}

void js_system_module::load(irccd&, std::shared_ptr<js_plugin> plugin)
{
    StackAssert sa(plugin->context());

    duk_get_global_string(plugin->context(), "Irccd");
    duk_push_object(plugin->context());
    duk_put_function_list(plugin->context(), -1, functions);
    duk_put_prop_string(plugin->context(), -2, "System");
    duk_pop(plugin->context());
}

} // !irccd
