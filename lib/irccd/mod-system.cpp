/*
 * js-system.cpp -- Irccd.System API
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

#include <chrono>
#include <cstdlib>
#include <thread>

#include "sysconfig.hpp"

#if defined(HAVE_POPEN)
#  include <cstdio>
#endif

#include "mod-file.hpp"
#include "mod-irccd.hpp"
#include "mod-system.hpp"
#include "plugin-js.hpp"
#include "system.hpp"

namespace irccd {

namespace {

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
 */
int env(duk::Context *ctx)
{
	duk::push(ctx, sys::env(duk::get<std::string>(ctx, 0)));

	return 1;
}

/*
 * Function: Irccd.System.exec(cmd)
 * ------------------------------------------------------------------
 *
 * Execute a system command.
 *
 * Arguments:
 *   - cmd, the command to execute.
 */
int exec(duk::Context *ctx)
{
	std::system(duk::get<const char *>(ctx, 0));

	return 0;
}

/*
 * Function: Irccd.System.home()
 * ------------------------------------------------------------------
 *
 * Get the operating system user's home.
 *
 * Returns:
 *   The user home directory.
 */
int home(duk::Context *ctx)
{
	duk::push(ctx, sys::home());

	return 1;
}

/*
 * Function: Irccd.System.name()
 * ------------------------------------------------------------------
 *
 * Get the operating system name.
 *
 * Returns:
 *   The system name.
 */
int name(duk::Context *ctx)
{
	duk::push(ctx, sys::name());

	return 1;
}

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
 *   A Irccd.File object.
 * Throws
 *   - Irccd.SystemError on failures.
 */
int popen(duk::Context *ctx)
{
	auto fp = ::popen(duk::require<const char *>(ctx, 0), duk::require<const char *>(ctx, 1));

	if (fp == nullptr)
		duk::raise(ctx, SystemError{});

	duk::push(ctx, new File(fp, [] (std::FILE *fp) { ::pclose(fp); }));

	return 1;
}

#endif // !HAVE_POPEN

/*
 * Function: Irccd.System.sleep(delay)
 * ------------------------------------------------------------------
 *
 * Sleep the main loop for the specific delay in seconds.
 */
int sleep(duk::Context *ctx)
{
	std::this_thread::sleep_for(std::chrono::seconds(duk::get<int>(ctx, 0)));

	return 0;
}

/*
 * Function: Irccd.System.ticks()
 * ------------------------------------------------------------------
 *
 * Get the number of milliseconds since irccd was started.
 *
 * Returns:
 *   The number of milliseconds.
 */
int ticks(duk::Context *ctx)
{
	duk::push(ctx, static_cast<int>(sys::ticks()));

	return 1;
}

/*
 * Function: Irccd.System.usleep(delay)
 * ------------------------------------------------------------------
 *
 * Sleep the main loop for the specific delay in microseconds.
 */
int usleep(duk::Context *ctx)
{
	std::this_thread::sleep_for(std::chrono::microseconds(duk::get<int>(ctx, 0)));

	return 0;
}

/*
 * Function: Irccd.System.uptime()
 * ------------------------------------------------------------------
 *
 * Get the system uptime.
 *
 * Returns:
 *   The system uptime.
 */
int uptime(duk::Context *ctx)
{
	duk::push<int>(ctx, sys::uptime());

	return 0;
}

/*
 * Function: Irccd.System.version()
 * ------------------------------------------------------------------
 *
 * Get the operating system version.
 *
 * Returns:
 *   The system version.
 */
int version(duk::Context *ctx)
{
	duk::push(ctx, sys::version());

	return 1;
}

const duk::FunctionMap functions{
	{ "env",	{ env,		1	} },
	{ "exec",	{ exec,		1	} },
	{ "home",	{ home,		0	} },
	{ "name",	{ name,		0	} },
#if defined(HAVE_POPEN)
	{ "popen",	{ popen,	2	} }, 
#endif
	{ "sleep",	{ sleep,	1	} },
	{ "ticks",	{ ticks,	0	} },
	{ "uptime",	{ uptime,	0	} },
	{ "usleep",	{ usleep,	1	} },
	{ "version",	{ version,	0	} }
};

} // !namespace

SystemModule::SystemModule() noexcept
	: Module("Irccd.System")
{
}

void SystemModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Object{});
	duk::put(plugin.context(), functions);
	duk::putProperty(plugin.context(), -2, "System");
	duk::pop(plugin.context());
}

} // !irccd
