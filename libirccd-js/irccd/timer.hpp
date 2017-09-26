/*
 * timer.hpp -- threaded timers
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

#ifndef IRCCD_TIMER_HPP
#define IRCCD_TIMER_HPP

/**
 * \file timer.hpp
 * \brief Provides interval based timers for JavaScript
 */

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "signals.hpp"
#include "sysconfig.hpp"

namespace irccd {

/**
 * \brief Timer class
 *
 * A timer is a thread object that emits a signal periodically or just one time. It is perfectly pausable and resumable
 * to reuse the same object.
 *
 * The delay is configured in milliseconds and the user has choice to use any
 * delay needed.
 *
 * We use a condition variable to wait for the specified delay unless the timer
 * must be stopped.
 */
class timer {
public:
    /**
     * \brief Type of timer
     */
    enum class type {
        single,             //!< The timer ends after execution
        repeat              //!< The timer loops
    };

    /**
     * Signal: onSignal
     * ----------------------------------------------------------
     *
     * Called when the timeout expires.
     */
    Signal<> on_signal;

    /**
     * Signal: onEnd
     * ----------------------------------------------------------
     *
     * Called when the timeout ends.
     */
    Signal<> on_end;

private:
    enum class state {
        paused,
        running,
        stopped
    };

    type type_;
    unsigned delay_;

    // Thread management.
    std::atomic<state> state_{state::paused};
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread thread_;

    void run();

public:
    /**
     * Timer constructor.
     *
     * The timer is not started, use start().
     *
     * \param type the timer type
     * \param delay the delay in milliseconds
     * \post isRunning() returns false
     */
    timer(type type, unsigned delay) noexcept;

    /**
     * Destructor, closes the thread.
     *
     * \pre stop() must have been called.
     */
    virtual ~timer();

    /**
     * Start the thread.
     *
     * \pre isRunning() must return false
     * \pre onSignal() must have been called
     * \pre onEnd() must have been called
     * \note Thread-safe
     */
    void start();

    /**
     * Stop the timer, may be used by the user to stop it.
     *
     * \note Thread-safe
     */
    void stop();

    /**
     * Get the type of timer.
     *
     * \return the type.
     */
    inline type get_type() const noexcept
    {
        return type_;
    }

    /**
     * Tells if the timer has still a running thread.
     *
     * \return true if still alive
     * \note Thread-safe
     */
    inline bool is_running() const noexcept
    {
        return state_ == state::running;
    }
};

} // !irccd

#endif // !IRCCD_TIMER_HPP
