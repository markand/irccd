/*
 * js-logger.h -- Irccd.Logger API
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

#include <logger.h>

#include "js-logger.h"

namespace irccd {

namespace {

int print(js::Context &ctx, std::ostream &out)
{
	/*
	 * Get the message before we start printing stuff to avoid
	 * empty lines.
	 */
	out << "plugin ";
	out << ctx.getGlobal<std::string>("\xff""\xff""name") << ": ";
	out << ctx.get<std::string>(0) << std::endl;

	return 0;
}

/*
 * Function: Irccd.Logger.info(message)
 * --------------------------------------------------------
 *
 * Write a verbose message.
 *
 * Arguments:
 *   - message, the message
 */
int info(js::Context &ctx)
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
 *   - message, the warning
 */
int warning(js::Context &ctx)
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
 *   - message, the message
 */
int debug(js::Context &ctx)
{
	return print(ctx, log::debug());
}

const js::FunctionMap functions{
	{ "info",	{ info,		1 } },
	{ "warning",	{ warning,	1 } },
	{ "debug",	{ debug,	1 } },
};

} // !namespace

void loadJsLogger(js::Context &ctx)
{
	ctx.getGlobal<void>("Irccd");
	ctx.push(js::Object{});
	ctx.push(functions);
	ctx.putProperty(-2, "Logger");
	ctx.pop();
}

} // !irccd
