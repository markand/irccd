/*
 * js-irccd.cpp -- Irccd API
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

#include <irccd-config.h>

#include "js-irccd.h"

namespace irccd {

SystemError::SystemError()
	: m_errno(errno)
	, m_message(std::strerror(m_errno))
{
}

SystemError::SystemError(int e, std::string message)
	: m_errno(e)
	, m_message(std::move(message))
{
}

void SystemError::create(js::Context &ctx) const
{
	ctx.getGlobal<void>("Irccd");
	ctx.getProperty<void>(-1, "SystemError");
	ctx.push(m_errno);
	ctx.push(m_message);
	duk_new(ctx, 2);
	ctx.remove(-2);
}

int constructor(js::Context &ctx)
{
	ctx.push(js::This{});
	ctx.putProperty(-1, "errno", ctx.require<int>(0));
	ctx.putProperty(-1, "message", ctx.require<std::string>(1));
	ctx.putProperty(-1, "name", "SystemError");
	ctx.pop();

	return 0;
}

void loadJsIrccd(js::Context &ctx)
{
	/* Irccd */
	ctx.push(js::Object{});

	/* Version */
	ctx.push(js::Object{});
	ctx.putProperty(-1, "major", IRCCD_VERSION_MAJOR);
	ctx.putProperty(-1, "minor", IRCCD_VERSION_MINOR);
	ctx.putProperty(-1, "patch", IRCCD_VERSION_PATCH);
	ctx.putProperty(-2, "version");

	/* Create the SystemError that inherits from Error */
	ctx.push(js::Function{constructor, 2});

	/* Prototype */
	ctx.getGlobal<void>("Error");
	duk_new(ctx, 0);
	ctx.dup(-2);
	ctx.putProperty(-2, "constructor");
	ctx.putProperty(-2, "prototype");
	ctx.putProperty(-2, "SystemError");

	/* Set Irccd as global */
	ctx.putGlobal("Irccd");
}

} // !irccd
