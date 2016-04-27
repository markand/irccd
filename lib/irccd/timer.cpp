/*
 * timer.cpp -- threaded timers
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
#include <chrono>

#include "timer.hpp"

namespace irccd {

void Timer::run()
{
	while (m_state != Stopped) {
		std::unique_lock<std::mutex> lock(m_mutex);

		/* Wait in case the timer is paused */
		m_condition.wait(lock, [&] () {
			return m_state != Paused;
		});

		if (m_state != Running) {
			continue;
		}

		/* Wait the timer delay or the interrupt */
		m_condition.wait_for(lock, std::chrono::milliseconds(m_delay), [&] () {
			return m_state != Running;
		});

		if (m_state == Running) {
			/* Signal process */
			onSignal();

			if (m_type == TimerType::Single) {
				m_state = Stopped;
			}
		}
	}

	onEnd();
}

Timer::Timer(TimerType type, unsigned delay) noexcept
	: m_type(type)
	, m_delay(delay)
	, m_thread([this] () { run(); })
{
}

Timer::~Timer()
{
	assert(m_state != Running);

	try {
		m_state = Stopped;
		m_condition.notify_one();
		m_thread.join();
	} catch (...) {
	}
}

void Timer::start()
{
	assert(m_state != Running);

	m_state = Running;
	m_condition.notify_one();
}

void Timer::stop()
{
	m_state = Paused;
	m_condition.notify_one();
}

} // !irccd
