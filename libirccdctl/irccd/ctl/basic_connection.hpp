/*
 * basic_connection.hpp -- network based connection for controller
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

#ifndef IRCCD_CTL_BASIC_CONNECTION_HPP
#define IRCCD_CTL_BASIC_CONNECTION_HPP

/**
 * \file basic_connection.hpp
 * \brief Network based connection for controller.
 */

#include <irccd/network_stream.hpp>

#include "connection.hpp"

namespace irccd {

namespace ctl {

/**
 * \brief Network based connection for controller.
 *
 * This class implements recv and send functions for Boost.Asio based sockets,
 * the subclasses only need to implement a connect function.
 */
template <typename Socket>
class basic_connection : public connection {
protected:
    network_stream<Socket> stream_;

public:
    /**
     * Construct the network connection.
     *
     * \param args the arguments to pass to the socket
     */
    template <typename... Args>
    inline basic_connection(Args&&... args)
        : stream_(std::forward<Args>(args)...)
    {
    }

    /**
     * Tells if the stream has pending actions.
     *
     * \return true if receiving/sending
     */
    bool is_active() const noexcept
    {
        return stream_.is_active();
    }

    /**
     * Implements connection::recv using boost::asio::async_read_until.
     *
     * \param handler the handler
     */
    void recv(network_recv_handler handler) override
    {
        stream_.recv(std::move(handler));
    }

    /**
     * Implements connection::send using boost::asio::async_write.
     *
     * \param json the JSON message
     * \param handler the handler
     */
    void send(nlohmann::json json, network_send_handler handler) override
    {
        stream_.send(std::move(json), std::move(handler));
    }
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_BASIC_CONNECTION_HPP
