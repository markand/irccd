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

#include <cassert>
#include <cstdint>

#include "js.hpp"
#include "plugin-js.hpp"

namespace irccd {

namespace duk {

template <>
class TypeTraits<Timer> {
public:
	static std::string name()
	{
		return "\xff""\xff""Timer";
	}

	static std::vector<std::string> inherits()
	{
		return {};
	}
};

} // !duk

namespace {

/*
 * Method: Timer.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method
 * is a no-op.
 */
duk::Ret start(duk::ContextPtr ctx)
{
	auto timer = duk::self<duk::Shared<Timer>>(ctx);

	if (!timer->isRunning()) {
		timer->start();
	}

	return 0;
}

/*
 * Method: Timer.stop()
 * --------------------------------------------------------
 *
 * Stop the timer.
 */
duk::Ret stop(duk::ContextPtr ctx)
{
	auto timer = duk::self<duk::Shared<Timer>>(ctx);

	if (timer->isRunning()) {
		timer->stop();
	}

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
duk::Ret constructor(duk::ContextPtr ctx)
{
	int type = duk::require<int>(ctx, 0);
	int delay = duk::require<int>(ctx, 1);

	if (!duk::is<duk::Function>(ctx, 2)) {
		duk::raise(ctx, duk::TypeError("missing callback function"));
	}

	auto timer = std::make_shared<Timer>(static_cast<TimerType>(type), delay);

	/* Add this timer to the underlying plugin */
	duk::getGlobal<duk::RawPointer<JsPlugin>>(ctx, "\xff""\xff""plugin")->addTimer(timer);

	/* Construct object */
	duk::construct(ctx, duk::Shared<Timer>{timer});

	/* Now store the JavaScript function to call */
	duk::dup(ctx, 2);
	duk::putGlobal(ctx, "\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));

	return 0;
}

const duk::Map<int> constants{
	{ "Single",	static_cast<int>(TimerType::Single) },
	{ "Repeat",	static_cast<int>(TimerType::Repeat) },
};

} // !namespace

void loadJsTimer(duk::ContextPtr ctx) noexcept
{
	duk::StackAssert sa(ctx);

	duk::getGlobal<void>(ctx, "Irccd");
	duk::push(ctx, duk::Function{constructor, 3});
	duk::push(ctx, constants);
	duk::push(ctx, duk::Object{});
	duk::push(ctx, methods);
	duk::putProperty(ctx, -2, "prototype");
	duk::putProperty(ctx, -2, "Timer");
	duk::pop(ctx);
}

} // !irccd
