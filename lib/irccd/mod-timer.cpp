/*
 * js-timer.cpp -- Irccd.Timer API
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

#include <format.h>

#include "irccd.hpp"
#include "logger.hpp"
#include "mod-timer.hpp"
#include "mod-plugin.hpp"
#include "plugin-js.hpp"
#include "timer.hpp"

using namespace fmt::literals;

namespace irccd {

namespace {

const std::string Signature{"\xff""\xff""irccd-timer-ptr"};
const std::string CallbackTable{"\xff""\xff""irccd-timer-callbacks"};

} // !namespace

namespace duk {

template <>
class TypeTraits<std::shared_ptr<Timer>> {
public:
	static inline void construct(duk::Context *ctx, std::shared_ptr<Timer> timer)
	{
		duk::StackAssert sa(ctx);

		duk::push(ctx, duk::This());
		duk::putProperty<void *>(ctx, -1, Signature, new std::shared_ptr<Timer>(std::move(timer)));
		duk::pop(ctx);
	}

	static inline std::shared_ptr<Timer> require(duk::Context *ctx, duk::Index index)
	{
		auto ptr = static_cast<std::shared_ptr<Timer> *>(duk::getProperty<void *>(ctx, index, Signature));

		if (!ptr)
			duk::raise(ctx, DUK_ERR_TYPE_ERROR, "not a Timer object");

		return *ptr;
	}
};

} // !duk

namespace {

void handleSignal(std::weak_ptr<JsPlugin> ptr, std::string key)
{
	auto plugin = ptr.lock();

	if (!plugin)
		return;

	auto irccd = duk::getGlobal<Irccd *>(plugin->context(), "\xff""\xff""irccd");

	irccd->post([plugin, key] (Irccd &) {
		duk::StackAssert sa(plugin->context());

		duk::getGlobal<void>(plugin->context(), CallbackTable);
		duk::getProperty<void>(plugin->context(), -1, key);
		duk::remove(plugin->context(), -2);

		if (duk::is<duk::Function>(plugin->context(), -1)) {
			if (duk::pcall(plugin->context()) != 0)
				log::warning("plugin {}: {}"_format(plugin->name(), duk::exception(plugin->context(), -1).stack));
			else
				duk::pop(plugin->context());
		} else
			duk::pop(plugin->context());
	});
}

/*
 * Method: Timer.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method is a no-op.
 */
duk::Ret start(duk::Context *ctx)
{
	auto timer = duk::self<std::shared_ptr<Timer>>(ctx);

	if (!timer->isRunning())
		timer->start();

	return 0;
}

/*
 * Method: Timer.stop()
 * --------------------------------------------------------
 *
 * Stop the timer.
 */
duk::Ret stop(duk::Context *ctx)
{
	auto timer = duk::self<std::shared_ptr<Timer>>(ctx);

	if (timer->isRunning())
		timer->stop();

	return 0;
}

const duk::FunctionMap methods{
	{ "start",	{ start,	0 } },
	{ "stop",	{ stop,		0 } }
};

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
duk::Ret constructor(duk::Context *ctx)
{
	// Check parameters.
	auto type = duk::require<int>(ctx, 0);
	auto delay = duk::require<int>(ctx, 1);

	if (type < static_cast<int>(TimerType::Single) || type > static_cast<int>(TimerType::Repeat))
		duk::raise(ctx, DUK_ERR_TYPE_ERROR, "invalid timer type");
	if (delay < 0)
		duk::raise(ctx, DUK_ERR_TYPE_ERROR, "negative delay given");
	if (!duk::is<duk::Function>(ctx, 2))
		duk::raise(ctx, DUK_ERR_TYPE_ERROR, "missing callback function");

	// Construct the timer in 'this'.
	auto timer = std::make_shared<Timer>(static_cast<TimerType>(type), delay);
	auto plugin = duk::TypeTraits<std::shared_ptr<JsPlugin>>::get(ctx);
	auto hash = std::to_string(reinterpret_cast<std::uintptr_t>(timer.get()));

	timer->onSignal.connect(std::bind(handleSignal, std::weak_ptr<JsPlugin>(plugin), hash));

	// Set a finalizer that closes the timer.
	duk::construct(ctx, std::shared_ptr<Timer>(std::move(timer)));
	duk::push(ctx, duk::This());
	duk::putProperty(ctx, -1, "\xff""\xff""timer-key", hash);
	duk::push(ctx, duk::Function{[] (duk::Context *ctx) -> duk::Ret {
		duk::StackAssert sa(ctx);

		duk::require<std::shared_ptr<Timer>>(ctx, 0)->stop();
		delete static_cast<std::shared_ptr<Timer> *>(duk::getProperty<void *>(ctx, 0, Signature));
		duk::getGlobal<void>(ctx, CallbackTable);
		duk::deleteProperty(ctx, -1, duk::getProperty<std::string>(ctx, 0, "\xff""\xff""timer-key"));
		duk::pop(ctx);
		log::debug("plugin: timer destroyed");

		return 0;
	}, 1});
	duk::setFinalizer(ctx, -2);

	// Save a callback function into the callback table.
	duk::getGlobal<void>(ctx, CallbackTable);
	duk::dup(ctx, 2);
	duk::putProperty(ctx, -2, hash);
	duk::pop(ctx);

	return 0;
}

const duk::Map<int> constants{
	{ "Single",	static_cast<int>(TimerType::Single) },
	{ "Repeat",	static_cast<int>(TimerType::Repeat) },
};

} // !namespace

TimerModule::TimerModule() noexcept
	: Module("Irccd.Timer")
{
}

void TimerModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Function{constructor, 3});
	duk::put(plugin.context(), constants);
	duk::push(plugin.context(), duk::Object{});
	duk::put(plugin.context(), methods);
	duk::putProperty(plugin.context(), -2, "prototype");
	duk::putProperty(plugin.context(), -2, "Timer");
	duk::pop(plugin.context());
	duk::putGlobal(plugin.context(), CallbackTable, duk::Object{});
}

} // !irccd
