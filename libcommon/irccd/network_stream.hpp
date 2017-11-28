/*
 * network_stream.hpp -- base shared network stream
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

#ifndef IRCCD_COMMON_NETWORK_STREAM_HPP
#define IRCCD_COMMON_NETWORK_STREAM_HPP

/**
 * \file network_stream.cpp
 * \brief Base shared network stream.
 */

#include "sysconfig.hpp"

#include <deque>
#include <functional>
#include <string>
#include <utility>

#include <boost/asio.hpp>

#if defined(HAVE_SSL)
#   include <boost/asio/ssl.hpp>
#endif

#include <json.hpp>

#include "network_errc.hpp"

namespace irccd {

/**
 * Read handler.
 *
 * Call this function when a receive operation has finished on success or
 * failure.
 */
using network_recv_handler = std::function<void (boost::system::error_code, nlohmann::json)>;

/**
 * Send handler.
 *
 * Call this function when a send operation has finished on success or failure.
 */
using network_send_handler = std::function<void (boost::system::error_code)>;

/**
 * \brief Base shared network stream.
 *
 * This class can be used to perform I/O over a networking socket, it is
 * implemented as asynchronous operations over Boost.Asio.
 *
 * All recv/send operations are placed in a queue and performed when possible.
 */
template <typename Socket>
class network_stream {
private:
    using rbuffer_t = boost::asio::streambuf;
    using rqueue_t = std::deque<network_recv_handler>;
    using squeue_t = std::deque<std::pair<std::string, network_send_handler>>;

    Socket socket_;
    rbuffer_t rbuffer_;
    rqueue_t rqueue_;
    squeue_t squeue_;

    void rflush();
    void sflush();
    void do_recv(network_recv_handler);
    void do_send(const std::string&, network_send_handler);

public:
    /**
     * Construct the stream.
     *
     * \param args the arguments to pass to the Socket constructor
     */
    template <typename... Args>
    inline network_stream(Args&&... args)
        : socket_(std::forward<Args>(args)...)
    {
    }

    /**
     * Get the underlying socket.
     *
     * \return the socket
     */
    inline const Socket& socket() const noexcept
    {
        return socket_;
    }

    /**
     * Overloaded function.
     *
     * \return the socket
     */
    inline Socket& socket() noexcept
    {
        return socket_;
    }

    /**
     * Tells if receive operations are pending.
     *
     * \return true if receiving is in progress
     */
    inline bool is_receiving() const noexcept
    {
        return !rqueue_.empty();
    }

    /**
     * Tells if send operations are pending.
     *
     * \return true if sending is in progress
     */
    inline bool is_sending() const noexcept
    {
        return !squeue_.empty();
    }

    /**
     * Tells if there are any I/O pending.
     *
     * \return true if sending is in progress
     */
    inline bool is_active() const noexcept
    {
        return is_receiving() || is_sending();
    }

    /**
     * Request a receive operation.
     *
     * \pre handler != nullptr
     * \param handler the handler
     */
    void recv(network_recv_handler);

    /**
     * Request a send operation.
     *
     * \pre json.is_object()
     * \param json the json message
     * \param handler the optional handler
     */
    void send(nlohmann::json json, network_send_handler = nullptr);
};

template <typename Socket>
void network_stream<Socket>::do_recv(network_recv_handler handler)
{
    boost::asio::async_read_until(socket_, rbuffer_, "\r\n\r\n", [this, handler] (auto code, auto xfer) {
        if (code)
            handler(std::move(code), nullptr);
        else if (xfer == 0U)
            handler(network_errc::corrupt_message, nullptr);
        else {
            std::string str(
                boost::asio::buffers_begin(rbuffer_.data()),
                boost::asio::buffers_begin(rbuffer_.data()) + xfer - 4
            );

            // Remove early in case of errors.
            rbuffer_.consume(xfer);

            // TODO: catch nlohmann::json::parse_error when 3.0.0 is released.
            nlohmann::json message;

            try {
                message = nlohmann::json::parse(str);
            } catch (...) {}

            if (!message.is_object())
                handler(network_errc::invalid_message, nullptr);
            else
                handler(network_errc::no_error, std::move(message));
        }
    });
}

template <typename Socket>
void network_stream<Socket>::do_send(const std::string& str, network_send_handler handler)
{
    boost::asio::async_write(socket_, boost::asio::buffer(str), [handler] (auto code, auto xfer) {
        if (code)
            handler(std::move(code));
        else if (xfer == 0U)
            handler(network_errc::corrupt_message);
        else
            handler(network_errc::no_error);
    });
}

template <typename Socket>
void network_stream<Socket>::rflush()
{
    if (rqueue_.empty())
        return;

    do_recv([this] (auto code, auto json) {
        auto h = rqueue_.front();

        if (h)
            h(code, std::move(json));

        rqueue_.pop_front();

        if (!code)
            rflush();
    });
}

template <typename Socket>
void network_stream<Socket>::sflush()
{
    if (squeue_.empty())
        return;

    do_send(squeue_.front().first, [this] (auto code) {
        auto h = squeue_.front().second;

        if (h)
            h(code);

        squeue_.pop_front();

        if (!code)
            sflush();
    });
}

template <typename Socket>
void network_stream<Socket>::recv(network_recv_handler handler)
{
    auto in_progress = !rqueue_.empty();

    rqueue_.push_back(std::move(handler));

    if (!in_progress)
        rflush();
}

template <typename Socket>
void network_stream<Socket>::send(nlohmann::json json, network_send_handler handler)
{
    assert(json.is_object());

    auto in_progress = !squeue_.empty();

    squeue_.emplace_back(json.dump(0) + "\r\n\r\n", std::move(handler));

    if (!in_progress)
        sflush();
}

/**
 * \brief Typedef for TCP/IP socket.
 */
using ip_network_stream = network_stream<boost::asio::ip::tcp::socket>;

#if !defined(IRCCD_SYSTEM_WINDOWS)

/**
 * \brief Typedef for Unix socket.
 */
using local_network_stream = network_stream<boost::asio::local::stream_protocol::socket>;

#endif // !IRCCD_SYSTEM_WINDOWS

#if defined(HAVE_SSL)

/**
 * \brief Typedef for SSL sockets.
 */
using tls_network_stream = network_stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;

#endif // !HAVE_SSL

} // !irccd

#endif // IRCCD_COMMON_NETWORK_STREAM_HPP
