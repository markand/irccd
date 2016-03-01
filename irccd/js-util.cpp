/*
 * js-util.cpp -- Irccd.Util API
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

#include <libircclient.h>

#include <util.h>

#include "js-util.h"

namespace irccd {

namespace js {

/**
 * Read parameters for Irccd.Util.format function, the object is defined as follow:
 *
 * {
 *   date: the date object
 *   flags: the flags (not implemented yet)
 *   field1: a field to substitute in #{} pattern
 *   field2: a field to substitute in #{} pattern
 *   fieldn: ...
 * }
 */
template <>
class TypeInfo<util::Substitution> {
public:
	static util::Substitution get(Context &ctx, int index)
	{
		util::Substitution params;

		if (!ctx.is<Object>(index))
			return params;

		ctx.enumerate(index, 0, true, [&] (Context &) {
			if (ctx.get<std::string>(-2) == "date")
				params.time = static_cast<time_t>(ctx.get<double>(-1) / 1000);
			else
				params.keywords.insert({ctx.get<std::string>(-2), ctx.get<std::string>(-1)});
		});

		return params;
	}
};

} // !js

namespace {

/*
 * Function: Irccd.Util.format(text, parameters)
 * --------------------------------------------------------
 *
 * Format a string with templates.
 *
 * Arguments:
 *   - input, the text to update,
 *   - params, the parameters.
 * Returns:
 *   The converted text.
 */
int format(js::Context &ctx)
{
	try {
		ctx.push(util::format(ctx.get<std::string>(0), ctx.get<util::Substitution>(1)));
	} catch (const std::exception &ex) {
		ctx.raise(js::SyntaxError(ex.what()));
	}

	return 1;
}

/*
 * Function: Irccd.Util.splituser(ident)
 * --------------------------------------------------------
 *
 * Return the nickname part from a full username.
 *
 * Arguments:
 *   - ident, the full identity.
 * Returns:
 *   The nickname.
 */
int splituser(js::Context &ctx)
{
	const char *target = ctx.require<const char *>(0);
	char nick[32] = {0};

	irc_target_get_nick(target, nick, sizeof (nick) -1);
	ctx.push(std::string(nick));

	return 1;
}

/*
 * Function: Irccd.Util.splithost(ident)
 * --------------------------------------------------------
 *
 * Return the hostname part from a full username.
 *
 * Arguments:
 *   - ident, the full identity.
 * Returns:
 *   The hostname.
 */
int splithost(js::Context &ctx)
{
	const char *target = ctx.require<const char *>(0);
	char host[32] = {0};

	irc_target_get_host(target, host, sizeof (host) -1);
	ctx.push(std::string(host));

	return 1;
}

const js::FunctionMap functions{
	{ "format",		{ format,	DUK_VARARGS	} },
	{ "splituser",		{ splituser,	1		} },
	{ "splithost",		{ splithost,	1		} }
};

} // !namespace

void loadJsUtil(js::Context &ctx)
{
	ctx.getGlobal<void>("Irccd");
	ctx.push(js::Object{});
	ctx.push(functions);
	ctx.putProperty(-2, "Util");
	ctx.pop();
}

} // !irccd
