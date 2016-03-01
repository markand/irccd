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

#include "js.h"
#include "plugin.h"

namespace irccd {

class JsTimer : public Timer {
public:
	using Timer::Timer;

	~JsTimer()
	{
		stop();
	}

	static inline const char *name() noexcept
	{
		return "\xff""\xff""Timer";
	}
};

namespace {

/*
 * Method: Timer.start()
 * --------------------------------------------------------
 *
 * Start the timer. If the timer is already started the method
 * is a no-op.
 */
int start(js::Context &ctx)
{
	auto timer = ctx.self<js::Shared<JsTimer>>();

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
int stop(js::Context &ctx)
{
	auto timer = ctx.self<js::Shared<JsTimer>>();

	if (timer->isRunning())
		timer->stop();

	return 0;
}

const js::FunctionMap methods{
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
int constructor(js::Context &ctx)
{
	int type = ctx.require<int>(0);
	int delay = ctx.require<int>(1);

	if (!duk_is_callable(ctx, 2))
		ctx.raise(js::TypeError{"missing callback function"});

	auto timer = std::make_shared<JsTimer>(static_cast<TimerType>(type), delay);

	/* Add this timer to the underlying plugin */
	ctx.getGlobal<js::RawPointer<Plugin>>("\xff""\xff""plugin")->addTimer(timer);

	/* Construct object */
	ctx.construct(js::Shared<JsTimer>{timer});

	/* Now store the JavaScript function to call */
	ctx.dup(2);
	ctx.putGlobal("\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));

	return 0;
}

const js::Map<int> constants{
	{ "Single",	static_cast<int>(TimerType::Single) },
	{ "Repeat",	static_cast<int>(TimerType::Repeat) },
};

} // !namespace

void loadJsTimer(js::Context &ctx) noexcept
{
	ctx.getGlobal<void>("Irccd");

	/* Timer object */
	ctx.push(js::Function{constructor, 3});
	ctx.push(constants);

	/* Prototype */
	ctx.push(js::Object{});
	ctx.push(methods);
	ctx.putProperty(-1, "\xff""\xff""Timer", true);
	ctx.putProperty(-2, "prototype");

	/* Put Timer */
	ctx.putProperty(-2, "Timer");
	ctx.pop();
}

} // !irccd
