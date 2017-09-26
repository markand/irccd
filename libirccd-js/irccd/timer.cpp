/*
 * timer.cpp -- threaded timers
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <iostream>

#include "timer.hpp"

namespace irccd {

void timer::run()
{
    while (state_ != state::stopped) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait in case the timer is paused.
        condition_.wait(lock, [&] () {
            return state_ == state::running;
        });

        if (state_ != state::running)
            continue;

        // Wait the timer delay or the interrupt.
        condition_.wait_for(lock, std::chrono::milliseconds(delay_), [&] () {
            return state_ != state::running;
        });

        if (state_ == state::running) {
            // Signal process.
            on_signal();

            if (type_ == type::single)
                state_ = state::stopped;
        }
    }

    on_end();
}

timer::timer(type type, unsigned delay) noexcept
    : type_(type)
    , delay_(delay)
    , thread_(std::bind(&timer::run, this))
{
}

timer::~timer()
{
    assert(state_ != state::running);

    try {
        {
            std::lock_guard<std::mutex> lk(mutex_);

            state_ = state::stopped;
            condition_.notify_one();
        }

        thread_.join();
    } catch (...) {
    }
}

void timer::start()
{
    assert(state_ != state::running);

    {
        std::lock_guard<std::mutex> lk(mutex_);
        state_ = state::running;
    }

    condition_.notify_one();
}

void timer::stop()
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        state_ = state::paused;
    }

    condition_.notify_one();
}

} // !irccd
