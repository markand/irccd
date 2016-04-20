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

#include "js-util.hpp"
#include "util.hpp"

namespace irccd {

namespace duk {

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
class TypeTraits<util::Substitution> {
public:
	static util::Substitution get(ContextPtr ctx, int index)
	{
		util::Substitution params;

		if (!duk::is<Object>(ctx, index))
			return params;

		duk::enumerate(ctx, index, 0, true, [&] (ContextPtr) {
			if (duk::get<std::string>(ctx, -2) == "date")
				params.time = static_cast<time_t>(duk::get<double>(ctx, -1) / 1000);
			else
				params.keywords.insert({duk::get<std::string>(ctx, -2), duk::get<std::string>(ctx, -1)});
		});

		return params;
	}
};

} // !duk

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
duk::Ret format(duk::ContextPtr ctx)
{
	try {
		duk::push(ctx, util::format(duk::get<std::string>(ctx, 0), duk::get<util::Substitution>(ctx, 1)));
	} catch (const std::exception &ex) {
		duk::raise(ctx, duk::SyntaxError(ex.what()));
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
duk::Ret splituser(duk::ContextPtr ctx)
{
	const char *target = duk::require<const char *>(ctx, 0);
	char nick[32] = {0};

	irc_target_get_nick(target, nick, sizeof (nick) -1);
	duk::push(ctx, std::string(nick));

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
duk::Ret splithost(duk::ContextPtr ctx)
{
	const char *target = duk::require<const char *>(ctx, 0);
	char host[32] = {0};

	irc_target_get_host(target, host, sizeof (host) -1);
	duk::push(ctx, std::string(host));

	return 1;
}

const duk::FunctionMap functions{
	{ "format",		{ format,	DUK_VARARGS	} },
	{ "splituser",		{ splituser,	1		} },
	{ "splithost",		{ splithost,	1		} }
};

} // !namespace

void loadJsUtil(duk::ContextPtr ctx)
{
	duk::StackAssert sa(ctx);

	duk::getGlobal<void>(ctx, "Irccd");
	duk::push(ctx, duk::Object{});
	duk::push(ctx, functions);
	duk::putProperty(ctx, -2, "Util");
	duk::pop(ctx);
}

} // !irccd
