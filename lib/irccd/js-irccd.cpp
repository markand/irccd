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

void SystemError::raise(duk::ContextPtr ctx) const
{
	duk::getGlobal<void>(ctx, "Irccd");
	duk::getProperty<void>(ctx, -1, "SystemError");
	duk::push(ctx, m_errno);
	duk::push(ctx, m_message);
	duk::create(ctx, 2);
	duk::remove(ctx, -2);
}

duk::Ret constructor(duk::ContextPtr ctx)
{
	duk::push(ctx, duk::This{});
	duk::putProperty(ctx, -1, "errno", duk::require<int>(ctx, 0));
	duk::putProperty(ctx, -1, "message", duk::require<std::string>(ctx, 1));
	duk::putProperty(ctx, -1, "name", "SystemError");
	duk::pop(ctx);

	return 0;
}

void loadJsIrccd(duk::Context &ctx)
{
	duk::StackAssert sa(ctx);

	/* Irccd */
	duk::push(ctx, duk::Object{});

	/* Version */
	duk::push(ctx, duk::Object{});
	duk::putProperty(ctx, -1, "major", IRCCD_VERSION_MAJOR);
	duk::putProperty(ctx, -1, "minor", IRCCD_VERSION_MINOR);
	duk::putProperty(ctx, -1, "patch", IRCCD_VERSION_PATCH);
	duk::putProperty(ctx, -2, "version");

	/* Create the SystemError that inherits from Error */
	duk::push(ctx, duk::Function{constructor, 2});
	duk::getGlobal<void>(ctx, "Error");
	duk::push(ctx, duk::Function{[] (duk::ContextPtr) -> duk::Ret { return 0; }});
	duk::getProperty<void>(ctx, -2, "prototype");
	duk::putProperty(ctx, -2, "prototype");
	duk::create(ctx, 0);
	duk::putProperty(ctx, -3, "prototype");
	duk::pop(ctx);
	duk::dup(ctx, -1);
	duk::putProperty(ctx, -2, "constructor");
	duk::putProperty(ctx, -2, "SystemError");

	/* Set Irccd as global */
	duk::putGlobal(ctx, "Irccd");
}

} // !irccd
