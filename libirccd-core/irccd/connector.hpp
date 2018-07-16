/*
 * connector.hpp -- abstract connection interface
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_COMMON_CONNECTOR_HPP
#define IRCCD_COMMON_CONNECTOR_HPP

/**
 * \file connector.hpp
 * \brief Abstract connection interface.
 */

#include <functional>
#include <memory>
#include <system_error>

namespace irccd::io {

class stream;

/**
 * \brief Connect completion handler.
 */
using connect_handler = std::function<void (std::error_code, std::shared_ptr<stream>)>;

/**
 * \brief Abstract connection interface.
 *
 * This class is used to connect to a stream end point (usually sockets) in an
 * asynchronous manner.
 *
 * Derived class must implement non-blocking connect function.
 */
class connector {
public:
    /**
     * Default constructor.
     */
    connector() = default;

    /**
     * Virtual destructor defaulted.
     */
    virtual ~connector() = default;

    /**
     * Start asynchronous connect.
     *
     * Once the client is connected, the original acceptor must be kept until it
     * is destroyed.
     *
     * \pre another connect operation must not be running
     * \pre handler != nullptr
     * \param handler the handler
     */
    virtual void connect(connect_handler handler) = 0;
};

} // !irccd::io

#endif // !IRCCD_COMMON_CONNECTOR_HPP
