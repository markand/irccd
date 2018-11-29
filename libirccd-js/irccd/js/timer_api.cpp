/*
 * timer_api.cpp -- Irccd.timer API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <boost/asio.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include "irccd_api.hpp"
#include "plugin.hpp"
#include "plugin_api.hpp"
#include "timer_api.hpp"

namespace asio = boost::asio;

using irccd::daemon::bot;

namespace irccd::js {

namespace {

const std::string_view signature(DUK_HIDDEN_SYMBOL("Irccd.Timer"));
const std::string_view table(DUK_HIDDEN_SYMBOL("Irccd.Timer.callbacks"));

// {{{ timer

class timer : public std::enable_shared_from_this<timer> {
public:
	enum class type {
		single,
		repeat
	};

private:
	boost::asio::deadline_timer handle_;
	plugin& plugin_;

	std::string key_;
	type type_;
	int delay_;

	bool is_running_{false};
	bool is_waiting_{false};

	void handle();

public:
	timer(boost::asio::io_service&, plugin&, type, int);

	auto key() const noexcept -> const std::string&;

	void start();

	void stop();
};

void timer::handle()
{
	duk::stack_guard sa(plugin_.get_context());

	duk_push_global_stash(plugin_.get_context());
	duk_get_prop_string(plugin_.get_context(), -1, table.data());
	duk_remove(plugin_.get_context(), -2);
	duk_get_prop_string(plugin_.get_context(), -1, key_.c_str());
	duk_remove(plugin_.get_context(), -2);

	if (duk_pcall(plugin_.get_context(), 0)) {
		auto& log = duk::type_traits<bot>::self(plugin_.get_context()).get_log();

		log.warning(static_cast<const daemon::plugin&>(plugin_)) << "timer error:" << std::endl;
		log.warning(static_cast<const daemon::plugin&>(plugin_)) << "  " << duk::get_stack(plugin_.get_context(), -1).what() << std::endl;
	} else
		duk_pop(plugin_.get_context());
}

timer::timer(boost::asio::io_service& service, plugin& plugin, type type, int delay)
	: handle_(service)
	, plugin_(plugin)
	, type_(type)
	, delay_(delay)
{
}

auto timer::key() const noexcept -> const std::string&
{
	return key_;
}

void timer::start()
{
	if (is_waiting_)
		return;

	is_running_ = is_waiting_ = true;

	handle_.expires_from_now(boost::posix_time::milliseconds(delay_));
	handle_.async_wait([this] (auto code) {
		is_waiting_ = false;

		if (code) {
			is_running_ = false;
			return;
		}

		handle();

		if (is_running_ && type_ == type::repeat)
			start();
	});
}

void timer::stop()
{
	if (is_running_) {
		handle_.cancel();
		is_running_ = false;
	}
}

// }}}

// {{{ self

auto self(duk_context* ctx) -> timer*
{
	duk::stack_guard sa(ctx);

	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, signature.data());
	auto ptr = duk_to_pointer(ctx, -1);
	duk_pop_2(ctx);

	if (!ptr)
		duk_error(ctx, DUK_ERR_TYPE_ERROR, "not a Timer object");

	return static_cast<timer*>(ptr);
}

// }}}

// {{{ Irccd.Timer.prototype.start

/*
 * Method: Irccd.Timer.prototype.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method is a no-op.
 */
auto Timer_prototype_start(duk_context* ctx) -> duk_ret_t
{
	self(ctx)->start();

	return 0;
}

// }}}

// {{{ Irccd.Timer.prototype.stop

/*
 * Method: Irccd.Timer.prototype.stop()
 * --------------------------------------------------------
 *
 * Stop the timer.
 */
auto Timer_prototype_stop(duk_context* ctx) -> duk_ret_t
{
	self(ctx)->stop();

	return 0;
}

// }}}

// {{{ Irccd.Timer [destructor]

/*
 * Function: Irccd.Timer() [destructor]
 * ------------------------------------------------------------------
 *
 * Deleter the timer.
 */
auto Timer_destructor(duk_context* ctx) -> duk_ret_t
{
	duk::stack_guard sa(ctx);

	// Get timer from this.
	duk_get_prop_string(ctx, 0, signature.data());
	auto ptr = static_cast<timer*>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);

	// Remove callback from timer table.
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, table.data());
	duk_remove(ctx, -2);
	duk_del_prop_string(ctx, -1, ptr->key().c_str());
	duk_pop(ctx);

	delete ptr;

	return 0;
}

// }}}

// {{{ Irccd.Timer [constructor]

/*
 * Function: Irccd.Timer(type, delay, callback) [constructor]
 * --------------------------------------------------------
 *
 * Create a new timer object.
 *
 * Arguments:
 *   - type, the type of timer (Irccd.Timer.Single or Irccd.Timer.Repeat),
 *   - delay, the interval in milliseconds,
 *   - callback, the function to call.
 */
auto Timer_constructor(duk_context* ctx) -> duk_ret_t
{
	if (!duk_is_constructor_call(ctx))
		return 0;

	try {
		// Check parameters.
		const auto type = duk_require_int(ctx, 0);
		const auto delay = duk_require_int(ctx, 1);

		if (type < static_cast<int>(timer::type::single) || type > static_cast<int>(timer::type::repeat))
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
		if (delay < 0)
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "negative delay given");
		if (!duk_is_callable(ctx, 2))
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

		auto& plg = duk::type_traits<plugin>::self(ctx);
		auto& daemon = duk::type_traits<bot>::self(ctx);
		auto object = new timer(daemon.get_service(), plg, static_cast<timer::type>(type), delay);

		duk_push_this(ctx);
		duk_push_pointer(ctx, object);
		duk_put_prop_string(ctx, -2, signature.data());
		duk_push_c_function(ctx, Timer_destructor, 1);
		duk_set_finalizer(ctx, -2);
		duk_pop(ctx);

		// Store the function in a table to be called later.
		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, table.data());
		duk_remove(ctx, -2);
		duk_dup(ctx, 2);
		duk_put_prop_string(ctx, -2, object->key().c_str());
		duk_pop(ctx);
	} catch (const std::exception& ex) {
		duk::raise(ctx, ex);
	}

	return 0;
}

// }}}

const duk_function_list_entry methods[] = {
	{ "start",      Timer_prototype_start,  0               },
	{ "stop",       Timer_prototype_stop,   0               },
	{ nullptr,      nullptr,                0               }
};

const duk_number_list_entry constants[] = {
	{ "Single",     static_cast<int>(timer::type::single)   },
	{ "Repeat",     static_cast<int>(timer::type::repeat)   },
	{ nullptr,      0                                       }
};

} // !namespace

auto timer_api::get_name() const noexcept -> std::string_view
{
	return "Irccd.Timer";
}

void timer_api::load(bot&, std::shared_ptr<plugin> plugin)
{
	duk::stack_guard sa(plugin->get_context());

	duk_get_global_string(plugin->get_context(), "Irccd");
	duk_push_c_function(plugin->get_context(), Timer_constructor, 3);
	duk_put_number_list(plugin->get_context(), -1, constants);
	duk_push_object(plugin->get_context());
	duk_put_function_list(plugin->get_context(), -1, methods);
	duk_put_prop_string(plugin->get_context(), -2, "prototype");
	duk_put_prop_string(plugin->get_context(), -2, "Timer");
	duk_pop(plugin->get_context());
	duk_push_global_stash(plugin->get_context());
	duk_push_object(plugin->get_context());
	duk_put_prop_string(plugin->get_context(), -2, table.data());
	duk_pop(plugin->get_context());
}

} // !irccd::js
