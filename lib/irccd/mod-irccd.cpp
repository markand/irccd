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

#include "mod-irccd.hpp"
#include "plugin-js.hpp"
#include "sysconfig.hpp"

namespace irccd {

namespace {

duk_ret_t constructor(duk_context *ctx)
{
	duk_push_this(ctx);
	duk_push_int(ctx, duk_require_int(ctx, 0));
	duk_put_prop_string(ctx, -2, "errno");
	duk_push_string(ctx, duk_require_string(ctx, 1));
	duk_put_prop_string(ctx, -2, "message");
	duk_push_string(ctx, "SystemError");
	duk_put_prop_string(ctx, -2, "name");
	duk_pop(ctx);

	return 0;
}

} // !namespace

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

void SystemError::raise(duk_context *ctx) const
{
	StackAssert sa(ctx, 0);

	duk_get_global_string(ctx, "Irccd");
	duk_get_prop_string(ctx, -1, "SystemError");
	duk_remove(ctx, -2);
	duk_push_int(ctx, m_errno);
	dukx_push_std_string(ctx, m_message);
	duk_new(ctx, 2);
	duk_throw(ctx);
}

IrccdModule::IrccdModule() noexcept
	: Module("Irccd")
{
}

void IrccdModule::load(Irccd &irccd, JsPlugin &plugin)
{
	StackAssert sa(plugin.context());

	// Irccd.
	duk_push_object(plugin.context());

	// Version.
	duk_push_object(plugin.context());
	duk_push_int(plugin.context(), IRCCD_VERSION_MAJOR);
	duk_put_prop_string(plugin.context(), -2, "major");
	duk_push_int(plugin.context(), IRCCD_VERSION_MINOR);
	duk_put_prop_string(plugin.context(), -2, "minor");
	duk_push_int(plugin.context(), IRCCD_VERSION_PATCH);
	duk_put_prop_string(plugin.context(), -2, "patch");
	duk_put_prop_string(plugin.context(), -2, "version");

	// Create the SystemError that inherits from Error.
	duk_push_c_function(plugin.context(), constructor, 2);
	duk_push_object(plugin.context());
	duk_get_global_string(plugin.context(), "Error");
	duk_get_prop_string(plugin.context(), -1, "prototype");
	duk_remove(plugin.context(), -2);
	duk_set_prototype(plugin.context(), -2);
	duk_put_prop_string(plugin.context(), -2, "prototype");
	duk_put_prop_string(plugin.context(), -2, "SystemError");

	// Set Irccd as global.
	duk_put_global_string(plugin.context(), "Irccd");

	// Store global instance.
	duk_push_pointer(plugin.context(), &irccd);
	duk_put_global_string(plugin.context(), "\xff""\xff""irccd-ref");
}

Irccd &duk_get_irccd(duk_context *ctx)
{
	StackAssert sa(ctx);

	duk_get_global_string(ctx, "\xff""\xff""irccd-ref");
	Irccd *irccd = static_cast<Irccd *>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);

	return *irccd;
}

} // !irccd
