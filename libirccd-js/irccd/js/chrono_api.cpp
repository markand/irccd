/*
 * chrono_api.cpp -- Irccd.Chrono API
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include <boost/timer/timer.hpp>

#include "chrono_api.hpp"
#include "plugin.hpp"

using irccd::daemon::bot;

namespace irccd::js {

namespace {

const std::string_view signature(DUK_HIDDEN_SYMBOL("Irccd.Chrono"));

// {{{ self

auto self(duk_context* ctx) -> boost::timer::cpu_timer*
{
	duk::stack_guard sa(ctx);

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, signature.data());
	const auto ptr = static_cast<boost::timer::cpu_timer*>(duk_to_pointer(ctx, -1));
	duk_pop_2(ctx);

	if (!ptr)
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not an Chrono object");

	return ptr;
}

// }}}

// {{{ Irccd.Chrono.prototype.pause

/*
 * Method: Chrono.prototype.pause
 * ------------------------------------------------------------------
 *
 * Pause the timer, without resetting the current elapsed time stored.
 */
auto Chrono_prototype_pause(duk_context* ctx) -> duk_ret_t
{
	self(ctx)->stop();

	return 0;
}

// }}}

// {{{ Irccd.Chrono.prototype.resume

/*
 * Method: Irccd.Chrono.prototype.resume
 * ------------------------------------------------------------------
 *
 * Restart the timer without resetting the current elapsed time.
 */
auto Chrono_prototype_resume(duk_context* ctx) -> duk_ret_t
{
	self(ctx)->resume();

	return 0;
}

// }}}

// {{{ Irccd.Chrono.prototype.elapsed

/*
 * Method: Chrono.prototype.elapsed
 * ------------------------------------------------------------------
 *
 * Get the number of elapsed milliseconds.
 *
 * Returns:
 *   The time elapsed.
 */
auto Chrono_prototype_elapsed(duk_context* ctx) -> duk_ret_t
{
	duk_push_uint(ctx, self(ctx)->elapsed().wall / 1000000LL);

	return 1;
}

// }}}

// {{{ Irccd.Chrono.prototype.start

/*
 * Method: Chrono.prototype.start
 * ------------------------------------------------------------------
 *
 * Starts or restarts accumulating time.
 */
auto Chrono_prototype_start(duk_context* ctx) -> duk_ret_t
{
	self(ctx)->start();

	return 0;
}

// }}}

// {{{ Irccd.Chrono [constructor]

/*
 * Function: Irccd.Chrono [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new Chrono object.
 */
auto Chrono_constructor(duk_context* ctx) -> duk_ret_t
{
	duk_push_this(ctx);
	duk_push_pointer(ctx, new boost::timer::cpu_timer);
	duk_put_prop_string(ctx, -2, signature.data());
	duk_pop(ctx);

	return 0;
}

// }}}

// {{{ Irccd.Chrono [destructor]

/*
 * Function: Irccd.Chrono [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
auto Chrono_destructor(duk_context* ctx) -> duk_ret_t
{
	duk_get_prop_string(ctx, 0, signature.data());
	delete static_cast<boost::timer::cpu_timer*>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	duk_del_prop_string(ctx, 0, signature.data());

	return 0;
}

// }}}

// {{{ definitions

const duk_function_list_entry methods[] = {
	{ "elapsed",    Chrono_prototype_elapsed,       0 },
	{ "pause",      Chrono_prototype_pause,         0 },
	{ "resume",     Chrono_prototype_resume,        0 },
	{ "start",      Chrono_prototype_start,         0 },
	{ nullptr,      nullptr,                        0 }
};

// }}}

} // !namespace

// {{{ chrono_api

auto chrono_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Chrono";
}

void chrono_api::load(bot&, plugin& plugin)
{
	duk::stack_guard sa(plugin.get_context());

	duk_get_global_string(plugin.get_context(), "Irccd");
	duk_push_c_function(plugin.get_context(), Chrono_constructor, 0);
	duk_push_object(plugin.get_context());
	duk_put_function_list(plugin.get_context(), -1, methods);
	duk_push_c_function(plugin.get_context(), Chrono_destructor, 1);
	duk_set_finalizer(plugin.get_context(), -2);
	duk_put_prop_string(plugin.get_context(), -2, "prototype");
	duk_put_prop_string(plugin.get_context(), -2, "Chrono");
	duk_pop(plugin.get_context());
}

// }}}

} // !irccd::js
