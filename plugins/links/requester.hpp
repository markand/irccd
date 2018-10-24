/*
 * requester.hpp -- convenient HTTP get requester
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

#ifndef IRCCD_REQUESTER_HPP
#define IRCCD_REQUESTER_HPP

/**
 * \file requester.hpp
 * \brief Convenient HTTP get requester.
 */

#include <irccd/sysconfig.hpp>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

#if defined(IRCCD_HAVE_SSL)
#   include <boost/asio/ssl.hpp>
#endif

#include <cstddef>
#include <memory>
#include <string>
#include <system_error>
#include <variant>

#include "uri.hpp"

namespace irccd {

class server;

/**
 * \brief Convenient HTTP get requester.
 */
class requester : public std::enable_shared_from_this<requester> {
private:
    std::variant<
        std::monostate,
        boost::asio::ip::tcp::socket
#if defined(IRCCD_HAVE_SSL)
        , boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
#endif
    > socket_;

    std::size_t level_{0U};
    std::shared_ptr<server> server_;
    std::string channel_;
    std::string origin_;

    uri uri_;

#if defined(IRCCD_HAVE_SSL)
    boost::asio::ssl::context ctx_{boost::asio::ssl::context::sslv23};
#endif

    boost::beast::flat_buffer buffer_;
    boost::beast::http::request<boost::beast::http::empty_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
    boost::asio::deadline_timer timer_;
    boost::asio::ip::tcp::resolver resolver_;

    void notify(const std::string&);
    void parse();
    void handle_read(const std::error_code&);
    void read();
    void handle_write(const std::error_code&);
    void write();
    void handle_handshake(const std::error_code&);
    void handshake();
    void handle_connect(const std::error_code&);
    void connect(const boost::asio::ip::tcp::resolver::results_type&);
    void handle_resolve(const std::error_code&, const boost::asio::ip::tcp::resolver::results_type&);
    void resolve();
    void handle_timer(const std::error_code&);
    void timer();
    void start();

    requester(boost::asio::io_context&,
              std::shared_ptr<server>,
              std::string,
              std::string,
              uri,
              std::size_t);

public:
    /**
     * Start seeking for a title in the link
     *
     * \param ctx the IO context
     * \param sv the server
     * \param origin the originator
     * \param channel the channel
     * \param message the message text
     */
    static void run(boost::asio::io_context& ctx,
                    std::shared_ptr<server> sv,
                    std::string origin,
                    std::string channel,
                    std::string message);
};

} // !irccd

#endif // !IRCCD_REQUESTER_HPP
