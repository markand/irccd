/*
 * net_util.hpp -- network utilities for pollable objects
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

#ifndef IRCCD_COMMON_NET_UTIL_HPP
#define IRCCD_COMMON_NET_UTIL_HPP

/**
 * \file net_util.hpp
 * \brief Network utilities for pollable objects.
 */

#include "net.hpp"

namespace irccd {

/**
 * \brief Miscellaneous utilities for Pollable objects
 */
namespace net_util {

/**
 * \cond HIDDEN_SYMBOLS
 */

inline void prepare(fd_set &, fd_set &, net::Handle &) noexcept
{
}

/**
 * \endcond
 */

/**
 * Call prepare function for every Pollable objects.
 *
 * \param in the input set
 * \param out the output set
 * \param max the maximum handle
 * \param first the first Pollable object
 * \param rest the additional Pollable objects
 */
template <typename Pollable, typename... Rest>
inline void prepare(fd_set &in, fd_set &out, net::Handle &max, Pollable &first, Rest&... rest)
{
    first.prepare(in, out, max);
    prepare(in, out, max, rest...);
}

/**
 * \cond HIDDEN_SYMBOLS
 */

inline void sync(fd_set &, fd_set &) noexcept
{
}

/**
 * \endcond
 */

/**
 * Call sync function for every Pollable objects.
 *
 * \param in the input set
 * \param out the output set
 * \param first the first Pollable object
 * \param rest the additional Pollable objects
 */
template <typename Pollable, typename... Rest>
inline void sync(fd_set &in, fd_set &out, Pollable &first, Rest&... rest)
{
    first.sync(in, out);
    sync(in, out, rest...);
}

/**
 * Prepare and sync Pollable objects.
 *
 * \param timeout the timeout in milliseconds (< 0 means forever)
 * \param first the the first Pollable object
 * \param rest the additional Pollable objects
 */
template <typename Pollable, typename... Rest>
void poll(int timeout, Pollable &first, Rest&... rest)
{
    fd_set in, out;
    timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };

    FD_ZERO(&in);
    FD_ZERO(&out);

    net::Handle max = 0;

    prepare(in, out, max, first, rest...);

    if (select(max + 1, &in, &out, nullptr, timeout < 0 ? nullptr : &tv) < 0 && errno != EINTR) {
        throw std::runtime_error(std::strerror(errno));
    } else {
        sync(in, out, first, rest...);
    }
}

/**
 * Parse a network message from an input buffer and remove it from it.
 *
 * \param input the buffer, will be updated
 * \return the message or empty string if there is nothing
 */
inline std::string next_network(std::string& input)
{
    std::string result;
    std::string::size_type pos = input.find("\r\n\r\n");

    if ((pos = input.find("\r\n\r\n")) != std::string::npos) {
        result = input.substr(0, pos);
        input.erase(input.begin(), input.begin() + pos + 4);
    }

    return result;
}

} // !net_util

} // !irccd

#endif // !IRCCD_COMMON_NET_UTIL_HPP
