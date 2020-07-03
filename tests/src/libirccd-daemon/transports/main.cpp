/*
 * main.cpp -- test server object
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "transports"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/transport_server.hpp>

using namespace std::chrono_literals;

namespace irccd::daemon {

namespace {

BOOST_AUTO_TEST_CASE(fix_995)
{
	boost::asio::io_context ctx;
	boost::asio::deadline_timer t1(ctx);
	boost::asio::deadline_timer t2(ctx);
	boost::asio::ip::tcp::socket cl1(ctx);
	boost::asio::ip::tcp::socket cl2(ctx);

	/*
	 * a server that waits for authentication, the client does not send
	 * anything the handler will never be executed.
	 */
	auto acc = std::make_unique<ip_acceptor>(ctx, "*", 0, true, false);
	auto ep = acc->get_acceptor().local_endpoint();
	auto tpt = std::make_shared<transport_server>(std::move(acc));
	auto connected1 = false;
	auto connected2 = false;

	for (auto timer : {&t1, &t2}) {
		timer->expires_from_now(boost::posix_time::seconds(3));
		timer->async_wait([] (auto code) {
			if (code != boost::asio::error::operation_aborted)
				throw std::system_error(std::make_error_code(std::errc::timed_out));
		});
	}

	tpt->set_password("test");
	tpt->accept([] (auto, auto) {});
	cl1.async_connect(ep, [&connected1, &t1] (auto) {
		connected1 = true;
		t1.cancel();
	});
	cl2.async_connect(ep, [&connected2, &t2] (auto) {
		connected2 = true;
		t2.cancel();
	});

	while (!connected1 && !connected2) {
		ctx.reset();
		ctx.poll();
		std::this_thread::sleep_for(1s);
	}

	BOOST_TEST(connected1);
	BOOST_TEST(connected2);
}

} // !namespace

} // !irccd::daemon
