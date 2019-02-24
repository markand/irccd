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

#define BOOST_TEST_MODULE "transports"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/transport_server.hpp>

namespace asio = boost::asio;

namespace irccd::daemon {

namespace {

BOOST_AUTO_TEST_CASE(fix_995)
{
	asio::io_context ctx;
	asio::ip::tcp::socket cl1(ctx);
	asio::ip::tcp::socket cl2(ctx);

	/*
	 * a server that waits for authentication, the client does not send
	 * anything the handler will never be executed.
	 */
	auto acc = std::make_unique<ip_acceptor>(ctx, "*", 0, true, false);
	auto ep = acc->get_acceptor().local_endpoint();
	auto tpt = std::make_shared<transport_server>(std::move(acc));
	auto accepted = false;
	auto connected1 = false;
	auto connected2 = false;

	tpt->set_password("test");
	tpt->accept([&accepted] (auto, auto) {
		accepted = true;
	});
	cl1.async_connect(ep, [&connected1] (auto) {
		connected1 = true;
	});
	cl2.async_connect(ep, [&connected2] (auto) {
		connected2 = true;
	});

	ctx.run();

	BOOST_TEST(accepted);
	BOOST_TEST(connected1);
	BOOST_TEST(connected2);
}

} // !namespace

} // !irccd::daemon
