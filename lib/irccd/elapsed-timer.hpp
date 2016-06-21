/*
 * elapsed-timer.hpp -- measure elapsed time
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

#ifndef IRCCD_ELAPSED_TIMER_HPP
#define IRCCD_ELAPSED_TIMER_HPP

/**
 * \file elapsed-timer.hpp
 * \brief Measure elapsed time
 */

#include <chrono>

#include "sysconfig.hpp"

namespace irccd {

/**
 * \class ElapsedTimer
 * \brief Measure elapsed time
 *
 * This class provides an abstraction to measure elapsed time since the
 * construction of the object.
 *
 * It uses std::chrono::high_resolution_clock for more precision and uses
 * milliseconds only.
 */
class ElapsedTimer {
private:
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    TimePoint m_last;
    bool m_paused{false};
    unsigned m_elapsed{0};

public:
    /**
     * Construct the elapsed timer, start counting.
     */
    IRCCD_EXPORT ElapsedTimer() noexcept;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~ElapsedTimer() = default;

    /**
     * Put the timer on pause, the already elapsed time is stored.
     */
    IRCCD_EXPORT void pause() noexcept;

    /**
     * Restart the timer, does not reset it.
     */
    IRCCD_EXPORT void restart() noexcept;

    /**
     * Reset the timer to 0.
     */
    IRCCD_EXPORT void reset() noexcept;

    /**
     * Get the number of elapsed milliseconds.
     *
     * \return the milliseconds
     */
    IRCCD_EXPORT unsigned elapsed() noexcept;
};

} // !irccd

#endif // !IRCCD_ELAPSED_TIMER_HPP
