/*
 * main.cpp -- test dynlib_plugin
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

#define BOOST_TEST_MODULE "dynlib_plugin"
#include <boost/test/unit_test.hpp>

/*
 * For this test, we open a plugin written in C++ and pass a journal_server
 * class for each of the plugin function.
 *
 * Then we verify that the appropriate function has been called correctly.
 *
 * Functions load, unload and reload can not be tested though.
 */

#include <irccd.hpp>
#include <dynlib_plugin.hpp>
#include <journal_server.hpp>

namespace irccd {

class fixture {
protected:
    boost::asio::io_service service_;
    std::shared_ptr<journal_server> server_;
    std::shared_ptr<plugin> plugin_;
    irccd irccd_;

    inline fixture()
        : server_(std::make_shared<journal_server>(service_, "test"))
        , irccd_(service_)
    {
        plugin_ = dynlib_plugin_loader({CMAKE_CURRENT_BINARY_DIR}).find("test-plugin");

        if (!plugin_)
            throw std::runtime_error("test plugin not found");
    }
};

BOOST_FIXTURE_TEST_SUITE(dynlib_plugin_suite, fixture)

BOOST_AUTO_TEST_CASE(on_command)
{
    plugin_->on_command(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_command");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_connect)
{
    plugin_->on_connect(irccd_, {server_});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_connect");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_invite)
{
    plugin_->on_invite(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_invite");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_join)
{
    plugin_->on_join(irccd_, {server_, "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_join");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_kick)
{
    plugin_->on_kick(irccd_, {server_, "", "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_kick");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_message)
{
    plugin_->on_message(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_message");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_me)
{
    plugin_->on_me(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_me");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_mode)
{
    plugin_->on_mode(irccd_, {server_, "", "", "", "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_mode");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_names)
{
    plugin_->on_names(irccd_, {server_, "", {}});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_names");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_nick)
{
    plugin_->on_nick(irccd_, {server_, "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_nick");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_notice)
{
    plugin_->on_notice(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_notice");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_part)
{
    plugin_->on_part(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_part");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_topic)
{
    plugin_->on_topic(irccd_, {server_, "", "", ""});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_topic");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_CASE(on_whois)
{
    plugin_->on_whois(irccd_, {server_, {"", "", "", "", {}}});

    BOOST_TEST(server_->cqueue().size() == 1U);
    BOOST_TEST(server_->cqueue()[0]["command"].get<std::string>() == "message");
    BOOST_TEST(server_->cqueue()[0]["message"].get<std::string>() == "on_whois");
    BOOST_TEST(server_->cqueue()[0]["target"].get<std::string>() == "test");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
