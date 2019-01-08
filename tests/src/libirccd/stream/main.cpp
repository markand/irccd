/*
 * main.cpp -- test network classes
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "stream"
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <boost/predef/os.h>

#include <irccd/sysconfig.hpp>

#include <irccd/acceptor.hpp>
#include <irccd/connector.hpp>
#include <irccd/stream.hpp>

using boost::asio::io_service;
using boost::asio::ip::tcp;

#if defined(IRCCD_HAVE_SSL)
using boost::asio::ssl::context;
#endif

namespace irccd {

namespace {

class stream_fixture {
public:
	io_service service_;

	std::unique_ptr<acceptor> acceptor_;
	std::unique_ptr<connector> connector_;

	std::shared_ptr<stream> stream1_;
	std::shared_ptr<stream> stream2_;

	virtual auto create_acceptor() -> std::unique_ptr<acceptor> = 0;

	virtual auto create_connector() -> std::unique_ptr<connector> = 0;

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

class ip_stream_fixture : public stream_fixture {
private:
	tcp::endpoint endpoint_;

protected:
	/**
	 * \copydoc io_fixture::create_acceptor
	 */
	auto create_acceptor() -> std::unique_ptr<acceptor> override
	{
		tcp::endpoint endpoint(tcp::v4(), 0U);
		tcp::acceptor acceptor(service_, std::move(endpoint));

		endpoint_ = acceptor.local_endpoint();

		return std::make_unique<ip_acceptor>(service_, std::move(acceptor));
	}

	/**
	 * \copydoc io_fixture::create_connector
	 */
	auto create_connector() -> std::unique_ptr<connector> override
	{
		const auto hostname = "127.0.0.1";
		const auto port = std::to_string(endpoint_.port());

		return std::make_unique<ip_connector>(service_, hostname, port, true, false);
	}
};

#if defined(IRCCD_HAVE_SSL)

class tls_ip_stream_fixture : public stream_fixture {
private:
	tcp::endpoint endpoint_;

protected:
	/**
	 * \copydoc io_fixture::create_acceptor
	 */
	auto create_acceptor() -> std::unique_ptr<acceptor> override
	{
		context context(context::tlsv12);

		context.use_certificate_file(TESTS_SOURCE_DIR "/data/test.crt", context::pem);
		context.use_private_key_file(TESTS_SOURCE_DIR "/data/test.key", context::pem);

		tcp::endpoint endpoint(tcp::v4(), 0U);
		tcp::acceptor acceptor(service_, std::move(endpoint));

		endpoint_ = acceptor.local_endpoint();

		return std::make_unique<tls_acceptor<ip_acceptor>>(std::move(context), service_, std::move(acceptor));
	}

	/**
	 * \copydoc io_fixture::create_connector
	 */
	auto create_connector() -> std::unique_ptr<connector> override
	{
		context context(context::tlsv12);

		const auto hostname = "127.0.0.1";
		const auto port = std::to_string(endpoint_.port());

		return std::make_unique<tls_connector<ip_connector>>(std::move(context),
			service_, hostname, port, true, false);
	}
};

#endif // !IRCCD_HAVE_SSL

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

class local_stream_fixture : public stream_fixture {
private:
	const std::string path_{CMAKE_CURRENT_BINARY_DIR "/stream-test.sock"};

public:

	/**
	 * \copydoc io_fixture::create_acceptor
	 */
	auto create_acceptor() -> std::unique_ptr<acceptor> override
	{
		return std::make_unique<local_acceptor>(service_, path_);
	}

	/**
	 * \copydoc io_fixture::create_connector
	 */
	auto create_connector() -> std::unique_ptr<connector> override
	{
		return std::make_unique<local_connector>(service_, path_);
	}
};

#if defined(IRCCD_HAVE_SSL)

class tls_local_stream_fixture : public stream_fixture {
private:
	const std::string path_{CMAKE_CURRENT_BINARY_DIR "/stream-test.sock"};

public:

	/**
	 * \copydoc io_fixture::create_acceptor
	 */
	auto create_acceptor() -> std::unique_ptr<acceptor> override
	{
		context context(context::tlsv12);

		context.use_certificate_file(TESTS_SOURCE_DIR "/data/test.crt", context::pem);
		context.use_private_key_file(TESTS_SOURCE_DIR "/data/test.key", context::pem);

		return std::make_unique<tls_acceptor<local_acceptor>>(std::move(context), service_, path_);
	}

	/**
	 * \copydoc io_fixture::create_connector
	 */
	auto create_connector() -> std::unique_ptr<connector> override
	{
		return std::make_unique<tls_connector<local_connector>>(context(context::tlsv12), service_, path_);
	}
};

#endif // !IRCCD_HAVE_SSL

#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS

/**
 * List of fixtures to tests.
 */
using list = boost::mpl::list<
	ip_stream_fixture
#if defined(IRCCD_HAVE_SSL)
	, tls_ip_stream_fixture
#endif
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
	, local_stream_fixture
#	if defined(IRCCD_HAVE_SSL)
	, tls_local_stream_fixture
#	endif
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
	fixture.stream1_->recv([] (auto code, auto message) {
		BOOST_TEST(!code);
		BOOST_TEST(message.is_object());
		BOOST_TEST(message["abc"].template get<int>() == 123);
		BOOST_TEST(message["def"].template get<int>() == 456);
	});
	fixture.stream2_->send(message, [] (auto code) {
		BOOST_TEST(!code);
	});
	fixture.service_.run();
}

BOOST_AUTO_TEST_CASE_TEMPLATE(connection_reset, Test, list)
{
	Test fixture;

	fixture.init();
	fixture.stream1_->recv([] (auto code, auto message) {
		BOOST_TEST(code.value() == static_cast<int>(std::errc::connection_reset));
		BOOST_TEST(message.is_null());
	});
	fixture.stream2_ = nullptr;
	fixture.service_.run();
}

} // !namespace

} // !irccd
