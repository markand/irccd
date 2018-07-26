/*
 * main.cpp -- test irccdctl server-info
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

#define BOOST_TEST_MODULE "irccdctl server-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/server_cli_test.hpp>

namespace irccd {

namespace {

BOOST_FIXTURE_TEST_SUITE(server_info_suite, server_cli_test)

BOOST_AUTO_TEST_CASE(output)
{
    start();

    const auto [code, out, err] = exec({ "server-info", "test" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 10U);
    BOOST_TEST(err.size() == 0U);
    BOOST_TEST(out[0] == "Name           : test");
    BOOST_TEST(out[1] == "Host           : localhost");
    BOOST_TEST(out[2] == "Port           : 6667");
    BOOST_TEST(out[3] == "Ipv6           : null");
    BOOST_TEST(out[4] == "SSL            : null");
    BOOST_TEST(out[5] == "SSL verified   : null");
    BOOST_TEST(out[6] == "Channels       : ");
    BOOST_TEST(out[7] == "Nickname       : irccd");
    BOOST_TEST(out[8] == "User name      : irccd");
    BOOST_TEST(out[9] == "Real name      : IRC Client Daemon");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
