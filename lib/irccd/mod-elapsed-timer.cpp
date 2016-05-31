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
#include "mod-elapsed-timer.hpp"
#include "plugin-js.hpp"

namespace irccd {

namespace {

const std::string Signature{"\xff""\xff""irccd-elapsed-timer-ptr"};

} // !namespace

namespace duk {

template <>
class TypeTraits<ElapsedTimer *> {
public:
	static inline void construct(duk::Context *ctx, ElapsedTimer *timer)
	{
		duk::StackAssert sa(ctx);

		duk::push(ctx, duk::This());
		duk::putProperty<void *>(ctx, -1, Signature, timer);
		duk::pop(ctx);
	}

	static inline ElapsedTimer *require(duk::Context *ctx, Index index)
	{
		auto ptr = static_cast<ElapsedTimer *>(duk::getProperty<void *>(ctx, index, Signature));

		if (!ptr)
			duk::raise(ctx, DUK_ERR_TYPE_ERROR, "not an ElapsedTimer object");

		return ptr;
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
duk::Ret pause(duk::Context *ctx)
{
	duk::self<ElapsedTimer *>(ctx)->pause();

	return 0;
}

/*
 * Method: ElapsedTimer.reset
 * ------------------------------------------------------------------
 *
 * Reset the elapsed time to 0, the status is not modified.
 */
duk::Ret reset(duk::Context *ctx)
{
	duk::self<ElapsedTimer *>(ctx)->reset();

	return 0;
}

/*
 * Method: ElapsedTimer.restart
 * ------------------------------------------------------------------
 *
 * Restart the timer without resetting the current elapsed time.
 */
duk::Ret restart(duk::Context *ctx)
{
	duk::self<ElapsedTimer *>(ctx)->restart();

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
duk::Ret elapsed(duk::Context *ctx)
{
	duk::push(ctx, static_cast<int>(duk::self<ElapsedTimer *>(ctx)->elapsed()));

	return 1;
}

/*
 * Function: Irccd.ElapsedTimer() [constructor]
 * ------------------------------------------------------------------
 *
 * Construct a new ElapsedTimer object.
 */
duk::Ret constructor(duk::Context *ctx)
{
	duk::construct(ctx, new ElapsedTimer);

	return 0;
}

/*
 * Function: Irccd.ElapsedTimer() [destructor]
 * ------------------------------------------------------------------
 *
 * Delete the property.
 */
duk::Ret destructor(duk::Context *ctx)
{
	delete static_cast<ElapsedTimer *>(duk::getProperty<void *>(ctx, 0, Signature));

	duk::deleteProperty(ctx, 0, Signature);

	return 0;
}

const duk::FunctionMap methods{
	{ "elapsed",	{ elapsed,	0 } },
	{ "pause",	{ pause,	0 } },
	{ "reset",	{ reset,	0 } },
	{ "restart",	{ restart,	0 } }
};

} // !namespace

ElapsedTimerModule::ElapsedTimerModule() noexcept
	: Module("Irccd.ElapsedTimer")
{
}

void ElapsedTimerModule::load(Irccd &, JsPlugin &plugin)
{
	duk::StackAssert sa(plugin.context());

	duk::getGlobal<void>(plugin.context(), "Irccd");
	duk::push(plugin.context(), duk::Function{constructor, 0});
	duk::push(plugin.context(), duk::Object{});
	duk::put(plugin.context(), methods);
	duk::push(plugin.context(), duk::Function(destructor, 1));
	duk::setFinalizer(plugin.context(), -2);
	duk::putProperty(plugin.context(), -2, "prototype");
	duk::putProperty(plugin.context(), -2, "ElapsedTimer");
	duk::pop(plugin.context());
}

} // !irccd
