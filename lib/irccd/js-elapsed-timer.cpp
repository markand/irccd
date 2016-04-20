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

#include "elapsed-timer.hpp"
#include "js.hpp"

namespace irccd {

namespace duk {

template <>
class TypeTraits<ElapsedTimer> {
public:
	static inline std::string name()
	{
		return "\xff""\xff""ElapsedTimer";
	}

	static inline std::vector<std::string> inherits()
	{
		return {};
	}
};

} // !duk

namespace {

/*
 * Method: ElapsedTimer.pause
 * ------------------------------------------------------------------
 *
 * Pause the timer, without resetting the current elapsed time stored.
 */
duk::Ret pause(duk::ContextPtr ctx)
{
	duk::self<duk::Pointer<ElapsedTimer>>(ctx)->pause();

	return 0;
}

/*
 * Method: ElapsedTimer.reset
 * ------------------------------------------------------------------
 *
 * Reset the elapsed time to 0, the status is not modified.
 */
duk::Ret reset(duk::ContextPtr ctx)
{
	duk::self<duk::Pointer<ElapsedTimer>>(ctx)->reset();

	return 0;
}

/*
 * Method: ElapsedTimer.restart
 * ------------------------------------------------------------------
 *
 * Restart the timer without resetting the current elapsed time.
 */
duk::Ret restart(duk::ContextPtr ctx)
{
	duk::self<duk::Pointer<ElapsedTimer>>(ctx)->restart();

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
duk::Ret elapsed(duk::ContextPtr ctx)
{
	duk::push(ctx, (int)duk::self<duk::Pointer<ElapsedTimer>>(ctx)->elapsed());

	return 1;
}

/*
 * Function: Irccd.ElapsedTimer() [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new ElapsedTimer object.
 */
duk::Ret constructor(duk::ContextPtr ctx)
{
	duk::construct(ctx, duk::Pointer<ElapsedTimer>{new ElapsedTimer});

	return 0;
}

const duk::FunctionMap methods{
	{ "elapsed",	{ elapsed,	0 } },
	{ "pause",	{ pause,	0 } },
	{ "reset",	{ reset,	0 } },
	{ "restart",	{ restart,	0 } }
};

} // !namespace

void loadJsElapsedTimer(duk::ContextPtr ctx) noexcept
{
	duk::StackAssert sa(ctx);

	duk::getGlobal<void>(ctx, "Irccd");
	duk::push(ctx, duk::Function{constructor, 0});
	duk::push(ctx, duk::Object{});
	duk::push(ctx, methods);
	duk::putProperty(ctx, -2, "prototype");
	duk::putProperty(ctx, -2, "ElapsedTimer");
	duk::pop(ctx);
}

} // !irccd
