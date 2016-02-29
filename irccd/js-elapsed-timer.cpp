/*
 * js-elapsed-timer.cpp -- Irccd.ElapsedTimer API
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

#include <elapsed-timer.h>

#include "js.h"

namespace irccd {

class JsElapsedTimer : public ElapsedTimer {
public:
	using ElapsedTimer::ElapsedTimer;

	static inline const char *name() noexcept
	{
		return "\xff""\xff""ElapsedTimer";
	}
};

namespace {

/*
 * Method: ElapsedTimer.pause
 * ------------------------------------------------------------------
 *
 * Pause the timer, without resetting the current elapsed time stored.
 */
int pause(js::Context &ctx)
{
	ctx.self<js::Pointer<JsElapsedTimer>>()->pause();

	return 0;
}

/*
 * Method: ElapsedTimer.reset
 * ------------------------------------------------------------------
 *
 * Reset the elapsed time to 0, the status is not modified.
 */
int reset(js::Context &ctx)
{
	ctx.self<js::Pointer<JsElapsedTimer>>()->reset();

	return 0;
}

/*
 * Method: ElapsedTimer.restart
 * ------------------------------------------------------------------
 *
 * Restart the timer without resetting the current elapsed time.
 */
int restart(js::Context &ctx)
{
	ctx.self<js::Pointer<JsElapsedTimer>>()->restart();

	return 0;
}

/*
 * Method: ElapsedTimer.elapsed
 * ------------------------------------------------------------------
 *
 * Get the number of elapsed milliseconds.
 *
 * Returns:
 *   The time elapsed.
 */
int elapsed(js::Context &ctx)
{
	ctx.push<int>(ctx.self<js::Pointer<JsElapsedTimer>>()->elapsed());

	return 1;
}

/*
 * Function: Irccd.ElapsedTimer() [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new ElapsedTimer object.
 */
int constructor(js::Context &ctx)
{
	ctx.construct(js::Pointer<JsElapsedTimer>{new JsElapsedTimer});

	return 0;
}

const js::FunctionMap methods{
	{ "elapsed",	{ elapsed,	0 } },
	{ "pause",	{ pause,	0 } },
	{ "reset",	{ reset,	0 } },
	{ "restart",	{ restart,	0 } }
};

} // !namespace

void loadJsElapsedTimer(js::Context &ctx) noexcept
{
	ctx.getGlobal<void>("Irccd");

	/* Timer object */
	ctx.push(js::Function{constructor, 3});

	/* Prototype */
	ctx.push(js::Object{});
	ctx.push(methods);
	ctx.putProperty(-1, "\xff""\xff""ElapsedTimer", true);
	ctx.putProperty(-2, "prototype");

	/* Put Timer */
	ctx.putProperty(-2, "ElapsedTimer");
	ctx.pop();
}

} // !irccd
