/*
 * main.cpp -- test io classes
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

#define BOOST_TEST_MODULE "io"
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <boost/predef/os.h>

#include <irccd/sysconfig.hpp>

#include <irccd/socket_acceptor.hpp>
#include <irccd/socket_connector.hpp>
#include <irccd/socket_stream.hpp>

#if defined(IRCCD_HAVE_SSL)
#   include <irccd/tls_acceptor.hpp>
#   include <irccd/tls_connector.hpp>
#   include <irccd/tls_stream.hpp>
#endif // !IRCCD_HAVE_SSL

using boost::asio::io_service;
using boost::asio::ip::tcp;

#if defined(IRCCD_HAVE_SSL)
using boost::asio::ssl::context;
#endif

#if !BOOST_OS_WINDOWS
using boost::asio::local::stream_protocol;
#endif

namespace irccd {

class io_test {
public:
    io_service service_;

    std::unique_ptr<io::acceptor> acceptor_;
    std::unique_ptr<io::connector> connector_;

    std::shared_ptr<io::stream> stream1_;
    std::shared_ptr<io::stream> stream2_;

    virtual std::unique_ptr<io::acceptor> create_acceptor() = 0;

    virtual std::unique_ptr<io::connector> create_connector() = 0;

    void init()
    {
        acceptor_ = create_acceptor();
        connector_ = create_connector();

        acceptor_->accept([this] (auto code, auto stream) {
            if (code)
                throw std::system_error(code);

            stream1_ = std::move(stream);
        });
        connector_->connect([this] (auto code, auto stream) {
            if (code)
                throw std::system_error(code);

            stream2_ = std::move(stream);
        });

        service_.run();
        service_.reset();
    }
};

class ip_io_test : public io_test {
private:
    tcp::endpoint endpoint_;

protected:
    /**
     * \copydoc io_test::create_acceptor
     */
    std::unique_ptr<io::acceptor> create_acceptor() override
    {
        tcp::endpoint endpoint(tcp::v4(), 0U);
        tcp::acceptor acceptor(service_, std::move(endpoint));

        endpoint_ = acceptor.local_endpoint();

        return std::make_unique<io::ip_acceptor>(std::move(acceptor));
    }

    /**
     * \copydoc io_test::create_connector
     */
    std::unique_ptr<io::connector> create_connector() override
    {
        return std::make_unique<io::ip_connector>(service_, endpoint_);
    }
};

#if defined(IRCCD_HAVE_SSL)

class ssl_io_test : public io_test {
private:
    tcp::endpoint endpoint_;

protected:
    /**
     * \copydoc io_test::create_acceptor
     */
    std::unique_ptr<io::acceptor> create_acceptor() override
    {
        context context(context::sslv23);

        context.use_certificate_file(TESTS_SOURCE_DIR "/data/test.crt", context::pem);
        context.use_private_key_file(TESTS_SOURCE_DIR "/data/test.key", context::pem);

        tcp::endpoint endpoint(tcp::v4(), 0U);
        tcp::acceptor acceptor(service_, std::move(endpoint));

        endpoint_ = acceptor.local_endpoint();

        return std::make_unique<io::tls_acceptor<>>(std::move(context), std::move(acceptor));
    }

    /**
     * \copydoc io_test::create_connector
     */
    std::unique_ptr<io::connector> create_connector() override
    {
        return std::make_unique<io::tls_connector<>>(context(context::sslv23), service_, endpoint_);
    }
};

#endif // !IRCCD_HAVE_SSL

#if !BOOST_OS_WINDOWS

class local_io_test : public io_test {
public:
    /**
     * \copydoc io_test::create_acceptor
     */
    std::unique_ptr<io::acceptor> create_acceptor() override
    {
        std::remove(CMAKE_BINARY_DIR "/tmp/io-test.sock");

        stream_protocol::acceptor acceptor(service_, CMAKE_BINARY_DIR "/tmp/io-test.sock");

        return std::make_unique<io::local_acceptor>(std::move(acceptor));
    }

    /**
     * \copydoc io_test::create_connector
     */
    std::unique_ptr<io::connector> create_connector() override
    {
        return std::make_unique<io::local_connector>(service_, CMAKE_BINARY_DIR "/tmp/io-test.sock");
    }
};

#endif // !BOOST_OS_WINDOWS

/**
 * List of fixtures to tests.
 */
using list = boost::mpl::list<
    ip_io_test
#if defined(IRCCD_HAVE_SSL)
    , ssl_io_test
#endif
#if !BOOST_OS_WINDOWS
    , local_io_test
#endif
>;

BOOST_AUTO_TEST_CASE_TEMPLATE(invalid_argument, Test, list)
{
    Test fixture;

    const nlohmann::json message{
        { "abc", 123 },
        { "def", 456 }
    };

    fixture.init();
    fixture.stream1_->read([] (auto code, auto message) {
        BOOST_TEST(!code);
        BOOST_TEST(message.is_object());
        BOOST_TEST(message["abc"].template get<int>() == 123);
        BOOST_TEST(message["def"].template get<int>() == 456);
    });
    fixture.stream2_->write(message, [] (auto code) {
        BOOST_TEST(!code);
    });
    fixture.service_.run();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(network_down, Test, list)
{
    Test fixture;

    fixture.init();
    fixture.stream1_->read([] (auto code, auto message) {
        BOOST_TEST(code.value() == static_cast<int>(std::errc::not_connected));
        BOOST_TEST(message.is_null());
    });
    fixture.stream2_ = nullptr;
    fixture.service_.run();
}

} // !irccd
