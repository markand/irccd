/*
 * util.hpp -- some utilities
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

#ifndef IRCCD_UTIL_HPP
#define IRCCD_UTIL_HPP

/**
 * \file util.hpp
 * \brief Utilities.
 */

#include <algorithm>

namespace irccd {

/**
 * \brief Namespace for utilities.
 */
namespace util {

/**
 * Clamp the value between low and high.
 *
 * \param value the value
 * \param low the minimum value
 * \param high the maximum value
 * \return the value between minimum and maximum
 */
template <typename T>
constexpr T clamp(T value, T low, T high) noexcept
{
    return (value < high) ? std::max(value, low) : std::min(value, high);
}

/**
 * Use arguments to avoid compiler warnings about unused parameters.
 */
template <typename... Args>
inline void unused(Args&&...) noexcept
{
}

} // !util

} // !irccd

#endif // !IRCCD_UTIL_HPP
