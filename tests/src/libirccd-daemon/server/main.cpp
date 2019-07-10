/*
 * main.cpp -- test server object
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

#define BOOST_TEST_MODULE "server"
#include <boost/test/unit_test.hpp>

#include <cassert>
#include <ostream>
#include <string>
#include <system_error>

#include <irccd/daemon/server.hpp>

using std::errc;
using std::error_code;
using std::make_shared;
using std::move;
using std::ostream;
using std::shared_ptr;
using std::string;
using std::system_error;

using boost::asio::io_context;
using boost::asio::ip::tcp;
using boost::asio::streambuf;

using irccd::daemon::server;

BOOST_TEST_DONT_PRINT_LOG_VALUE(errc)
BOOST_TEST_DONT_PRINT_LOG_VALUE(server::state)

namespace irccd {

namespace {

class fixture {
protected:
	io_context context_;
	streambuf output_;
	tcp::acceptor acceptor_{context_};
	tcp::socket socket_{context_};
	shared_ptr<server> server_;

	fixture();

	void accept();
	void feed(string);
	void pair();
};

void fixture::accept()
{
	acceptor_.async_accept(socket_, [] (auto code) {
		if (code)
			throw system_error(move(code));
	});
}

void fixture::feed(string data)
{
	assert(server_->get_state() != server::state::disconnected);

	ostream out(&output_);

	out << data;
	out << std::flush;

	async_write(socket_, output_, [] (auto code, auto) {
		if (code)
			throw system_error(move(code));
	});
}

void fixture::pair()
{
	accept();

	server_->connect([] (auto code) {
		if (code)
			throw system_error(move(code));
	});

	context_.run();
	context_.reset();

	BOOST_TEST(server_->get_state() == server::state::identifying);
}

fixture::fixture()
{
	acceptor_.open(tcp::v4());
	acceptor_.bind(tcp::endpoint(tcp::v4(), 0U));
	acceptor_.listen(1);

	server_ = make_shared<server>(context_, "test", "127.0.0.1");
	server_->set_port(acceptor_.local_endpoint().port());
	server_->set_options(server::options::ipv4);
	server_->set_ping_timeout(3);
}

BOOST_FIXTURE_TEST_SUITE(fixture_suite, fixture)

BOOST_AUTO_TEST_SUITE(interrupts)

/*
 * This test checks that interrupting any pending operations will never
 * trigger the handlers.
 */
BOOST_AUTO_TEST_CASE(not_connected_yet)
{
	accept();

	server_->connect([] (auto) {
		BOOST_FAIL("handler called (not expected");
	});
	server_->disconnect();

	context_.run();
}

/*
 * This test checks that interrupting a connected server will never trigger
 * handlers.
 */
BOOST_AUTO_TEST_CASE(connected)
{
	pair();

	server_->recv([] (auto, auto) {
		BOOST_FAIL("handler called (not expected");
	});
	server_->disconnect();

	context_.run();
}

/*
 * This test checks that interrupting the wait timer will never trigger
 * handlers.
 */
BOOST_AUTO_TEST_CASE(waiting)
{
	server_->wait([] (auto) {
		BOOST_FAIL("handler called (not expected");
	});
	server_->disconnect();

	context_.run();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(timeout)
{
	pair();

	error_code result;

	server_->recv([&] (auto code, auto) {
		result = move(code);
	});

	context_.run();

	BOOST_TEST(result == errc::timed_out);
}

BOOST_AUTO_TEST_CASE(connection_reset)
{
	pair();

	error_code result;

	server_->recv([&] (auto code, auto) {
		result = move(code);
	});
	socket_.close();

	context_.run();

	BOOST_TEST(result == errc::connection_reset);
}

BOOST_AUTO_TEST_CASE(argument_list_too_long)
{
	pair();
	feed(string(2048, 'a'));

	error_code result;

	server_->recv([&] (auto code, auto) {
		result = move(code);
	});

	context_.run();

	BOOST_TEST(result == errc::argument_list_too_long);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
