/*
 * main.cpp -- test network_stream class
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

#define BOOST_TEST_MODULE "network-stream"
#include <boost/test/unit_test.hpp>

#include <irccd/network_stream.hpp>

using boost::asio::io_service;
using boost::asio::ip::tcp;

namespace irccd {

class network_stream_test {
protected:
    io_service service_;
    tcp::socket connection_{service_};
    ip_network_stream stream_{service_};

    network_stream_test()
    {
        // Bind to a random port.
        tcp::endpoint ep(tcp::v4(), 0);
        tcp::acceptor acceptor(service_, ep);

        acceptor.async_accept(connection_, [] (auto code) {
            if (code)
                throw boost::system::system_error(code);
        });
        stream_.get_socket().async_connect(acceptor.local_endpoint(), [] (auto code) {
            if (code)
                throw boost::system::system_error(code);
        });

        service_.run();
        service_.reset();
    }
};

BOOST_FIXTURE_TEST_SUITE(network_stream_test_suite, network_stream_test)

BOOST_AUTO_TEST_CASE(multiple_recv)
{
    const auto msg1 = nlohmann::json({{ "abc", 123 }}).dump(0) + "\r\n\r\n";
    const auto msg2 = nlohmann::json({{ "def", 456 }}).dump(0) + "\r\n\r\n";

    stream_.recv([] (auto code, auto message) {
        BOOST_TEST(!code);
        BOOST_TEST(message["abc"].template get<int>() == 123);
    });
    stream_.recv([] (auto code, auto message) {
        BOOST_TEST(!code);
        BOOST_TEST(message["def"].template get<int>() == 456);
    });

    boost::asio::async_write(connection_, boost::asio::buffer(msg1), [] (auto code, auto) {
        BOOST_TEST(!code);
    });
    boost::asio::async_write(connection_, boost::asio::buffer(msg2), [] (auto code, auto) {
        BOOST_TEST(!code);
    });

    service_.run();
}

BOOST_AUTO_TEST_CASE(multiple_send)
{
    boost::asio::streambuf input;

    stream_.send({{ "abc", 123 }}, [] (auto code) {
        BOOST_TEST(!code);
    });
    stream_.send({{ "def", 456 }}, [] (auto code) {
        BOOST_TEST(!code);
    });

    boost::asio::async_read_until(connection_, input, "\r\n\r\n", [&] (auto code, auto xfer) {
        BOOST_TEST(!code);

        const auto json = nlohmann::json::parse(std::string(
            boost::asio::buffers_begin(input.data()),
            boost::asio::buffers_begin(input.data()) + xfer - 4
        ));

        input.consume(xfer);

        BOOST_TEST(json["abc"].template get<int>() == 123);
    });

    service_.run();
    service_.reset();

    boost::asio::async_read_until(connection_, input, "\r\n\r\n", [&] (auto code, auto xfer) {
        BOOST_TEST(!code);

        const auto json = nlohmann::json::parse(std::string(
            boost::asio::buffers_begin(input.data()),
            boost::asio::buffers_begin(input.data()) + xfer - 4
        ));

        input.consume(xfer);

        BOOST_TEST(json["def"].template get<int>() == 456);
    });

    service_.run();
}

BOOST_AUTO_TEST_CASE(invalid_argument)
{
    const std::string msg1("not a json object\r\n\r\n");

    stream_.recv([] (auto code, auto message) {
        BOOST_TEST(code == boost::system::errc::invalid_argument);
        BOOST_TEST(message.is_null());
    });

    boost::asio::async_write(connection_, boost::asio::buffer(msg1), [] (auto code, auto) {
        BOOST_TEST(!code);
    });

    service_.run();
}

BOOST_AUTO_TEST_CASE(network_down)
{
    stream_.recv([] (auto code, auto message) {
        BOOST_TEST(code == boost::system::errc::network_down);
        BOOST_TEST(message.is_null());
    });

    connection_.close();
    service_.run();
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
