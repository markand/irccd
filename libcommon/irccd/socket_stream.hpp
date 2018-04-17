/*
 * socket_stream.hpp -- socket stream interface
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

#ifndef IRCCD_COMMON_SOCKET_STREAM_HPP
#define IRCCD_COMMON_SOCKET_STREAM_HPP

/**
 * \file socket_stream.hpp
 * \brief Socket stream interface.
 */

#include <irccd/sysconfig.hpp>

#include <cstddef>
#include <cassert>
#include <string>
#include <utility>

#include <boost/asio.hpp>

#include "stream.hpp"

namespace irccd {

namespace io {

/**
 * \cond HIDDEN_SYMBOLS
 */

namespace detail {

/**
 * Convert boost::system::error_code to std.
 *
 * \param code the error code
 * \return the std::error_code
 */
inline std::error_code convert(boost::system::error_code code) noexcept
{
    return std::error_code(code.value(), std::system_category());
}

} // !detail

/**
 * \endcond
 */

/**
 * \brief Socket implementation interface.
 * \tparam Socket the Boost.Asio compatible socket.
 *
 * This class reimplements stream for Boost.Asio sockets.
 */
template <typename Socket>
class socket_stream : public stream {
private:
    Socket socket_;
    boost::asio::streambuf input_;
    std::string output_;

#if !defined(NDEBUG)
    bool is_receiving_{false};
    bool is_sending_{false};
#endif

    void handle_read(boost::system::error_code, std::size_t, read_handler);
    void handle_write(boost::system::error_code, std::size_t, write_handler);

public:
    /**
     * Create the socket stream.
     *
     * \param args the Socket constructor arguments
     */
    template <typename... Args>
    inline socket_stream(Args&&... args)
        : socket_(std::forward<Args>(args)...)
    {
    }

    /**
     * Get the underlying socket.
     *
     * \return the socket
     */
    inline const Socket& get_socket() const noexcept
    {
        return socket_;
    }

    /**
     * Overloaded function
     *
     * \return the socket
     */
    inline Socket& get_socket() noexcept
    {
        return socket_;
    }

    /**
     * \copydoc stream::read
     */
    void read(read_handler handler) override;

    /**
     * \copydoc stream::write
     */
    void write(const nlohmann::json& json, write_handler handler) override;
};

template <typename Socket>
void socket_stream<Socket>::handle_read(boost::system::error_code code,
                                        std::size_t xfer,
                                        read_handler handler)
{
#if !defined(NDEBUG)
    is_receiving_ = false;
#endif

    if (xfer == 0U) {
        handler(make_error_code(std::errc::not_connected), nullptr);
        return;
    }
    if (code) {
        handler(detail::convert(code), nullptr);
        return;
    }

    // 1. Convert the buffer safely.
    std::string buffer;

    try {
        buffer = std::string(
            boost::asio::buffers_begin(input_.data()),
            boost::asio::buffers_begin(input_.data()) + xfer - /* \r\n\r\n */ 4
        );

        input_.consume(xfer);
    } catch (const std::bad_alloc&) {
        handler(make_error_code(std::errc::not_enough_memory), nullptr);
        return;
    }

    // 2. Convert to JSON.
    nlohmann::json doc;

    try {
        doc = nlohmann::json::parse(buffer);
    } catch (const std::exception&) {
        handler(make_error_code(std::errc::invalid_argument), nullptr);
        return;
    }

    if (!doc.is_object())
        handler(make_error_code(std::errc::invalid_argument), nullptr);
    else
        handler(std::error_code(), std::move(doc));
}

template <typename Socket>
void socket_stream<Socket>::handle_write(boost::system::error_code code,
                                         std::size_t xfer,
                                         write_handler handler)
{
#if !defined(NDEBUG)
    is_sending_ = false;
#endif

    if (xfer == 0)
        handler(make_error_code(std::errc::not_connected));
    else
        handler(detail::convert(code));
}

template <typename Socket>
void socket_stream<Socket>::read(read_handler handler)
{
#if !defined(NDEBUG)
    assert(!is_receiving_);
    assert(handler);

    is_receiving_ = true;
#endif

    boost::asio::async_read_until(get_socket(), input_, "\r\n\r\n", [this, handler] (auto code, auto xfer) {
        handle_read(code, xfer, std::move(handler));
    });
}

template <typename Socket>
void socket_stream<Socket>::write(const nlohmann::json& json, write_handler handler)
{
#if !defined(NDEBUG)
    assert(!is_sending_);
    assert(handler);

    is_sending_ = true;
#endif

    output_ = json.dump(0) + "\r\n\r\n";

    const auto buffer = boost::asio::buffer(output_.data(), output_.size());

    boost::asio::async_write(get_socket(), buffer, [this, handler] (auto code, auto xfer) {
        handle_write(code, xfer, std::move(handler));
    });
}

/**
 * Convenient TCP/IP stream type.
 */
using ip_stream = socket_stream<boost::asio::ip::tcp::socket>;

#if !defined(IRCCD_SYSTEM_WINDOWS)

/**
 * Convenient Unix stream type.
 */
using local_stream = socket_stream<boost::asio::local::stream_protocol::socket>;

#endif

} // !io

} // !irccd

#endif // !IRCCD_COMMON_SOCKET_STREAM_HPP
