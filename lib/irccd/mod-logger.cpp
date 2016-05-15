/*
 * mod-logger.cpp -- Irccd.Logger API
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

#include "mod-logger.hpp"
#include "logger.hpp"
#include "plugin-js.hpp"

namespace irccd {

namespace {

duk::Ret print(duk::ContextPtr ctx, std::ostream &out)
{
	/*
	 * Get the message before we start printing stuff to avoid
	 * empty lines.
	 */
	out << "plugin " << duk::getGlobal<std::string>(ctx, "\xff""\xff""name");
	out << ": " << duk::require<std::string>(ctx, 0) << std::endl;

	return 0;
}

/*
 * Function: Irccd.Logger.info(message)
 * --------------------------------------------------------
 *
 * Write a verbose message.
 *
 * Arguments:
 *   - message, the message.
 */
duk::Ret info(duk::ContextPtr ctx)
{
	return print(ctx, log::info());
}

/*
 * Function: Irccd.Logger.warning(message)
 * --------------------------------------------------------
 *
 * Write a warning message.
 *
 * Arguments:
 *   - message, the warning.
 */
duk::Ret warning(duk::ContextPtr ctx)
{
	return print(ctx, log::warning());
}

/*
 * Function: Logger.debug(message)
 * --------------------------------------------------------
 *
 * Write a debug message, only shown if irccd is compiled in debug.
 *
 * Arguments:
 *   - message, the message.
 */
duk::Ret debug(duk::ContextPtr ctx)
{
	return print(ctx, log::debug());
}

const duk::FunctionMap functions{
	{ "info",	{ info,		1 } },
	{ "warning",	{ warning,	1 } },
	{ "debug",	{ debug,	1 } }
};

} // !namespace

LoggerModule::LoggerModule() noexcept
	: Module("Irccd.Logger")
{
}

void LoggerModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Object{});
	duk::push(plugin.context(), functions);
	duk::putProperty(plugin.context(), -2, "Logger");
	duk::pop(plugin.context());
}

} // !irccd
