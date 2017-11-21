/*
 * network_connection.hpp -- network based connection for controller
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

#ifndef IRCCD_CTL_NETWORK_CONNECTION_HPP
#define IRCCD_CTL_NETWORK_CONNECTION_HPP

/**
 * \file network_connection.hpp
 * \brief Network based connection for controller.
 */

#include <boost/asio.hpp>

#include <irccd/network_errc.hpp>

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
class network_connection : public connection {
private:
    boost::asio::streambuf input_;

protected:
    /**
     * The underlying socket.
     */
    Socket socket_;

public:
    /**
     * Construct the network connection.
     *
     * \param args the arguments to pass to the socket
     */
    template <typename... Args>
    inline network_connection(Args&&... args)
        : socket_(std::forward<Args>(args)...)
    {
    }

    /**
     * Implements connection::recv using boost::asio::async_read_until.
     *
     * \param handler the handler
     */
    void recv(recv_t handler) override;

    /**
     * Implements connection::send using boost::asio::async_write.
     *
     * \param handler the handler
     */
    void send(const nlohmann::json& json, send_t) override;
};

template <typename Socket>
void network_connection<Socket>::recv(recv_t handler)
{
    boost::asio::async_read_until(socket_, input_, "\r\n\r\n", [handler, this] (auto code, auto xfer) {
        if (code || xfer == 0) {
            handler(code, nullptr);
            return;
        }

        std::string command{
            boost::asio::buffers_begin(input_.data()),
            boost::asio::buffers_begin(input_.data()) + xfer - 4
        };

        input_.consume(xfer);

        /*
         * Only catch parse error from JSON to avoid calling the handler twice
         * if the handler throws from itself.
         *
         * TODO: in json 3.0.0, you may catch nlohmann::json::parse_error
         * instead.
         */
        nlohmann::json json;

        try {
            json = nlohmann::json::parse(command);
        } catch (...) {
            handler(network_errc::invalid_message, nullptr);
        }

        if (!json.is_object())
            handler(network_errc::invalid_message, nullptr);
        else
            handler(network_errc::no_error, std::move(json));
    });
}

template <typename Socket>
void network_connection<Socket>::send(const nlohmann::json& message, send_t handler)
{
    auto data = std::make_shared<std::string>(message.dump(0) + "\r\n\r\n");

    boost::asio::async_write(socket_, boost::asio::buffer(*data), [handler, data, this] (auto code, auto) {
        handler(code);
    });
}

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_NETWORK_CONNECTION_HPP
