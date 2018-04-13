/*
 * main.cpp -- test dynlib_plugin
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

#define BOOST_TEST_MODULE "dynlib_plugin"
#include <boost/test/unit_test.hpp>

/*
 * For this test, we update internal plugin configuration each time a function
 * is called and check if it has been called correctly using get_config.
 */

#include <irccd/daemon/dynlib_plugin.hpp>
#include <irccd/daemon/irccd.hpp>

namespace irccd {

class fixture {
protected:
    boost::asio::io_service service_;
    std::shared_ptr<plugin> plugin_;
    irccd irccd_{service_};

    inline fixture()
    {
        plugin_ = dynlib_plugin_loader({CMAKE_CURRENT_BINARY_DIR}).find("test-plugin");

        if (!plugin_)
            throw std::runtime_error("test plugin not found");
    }
};

BOOST_FIXTURE_TEST_SUITE(dynlib_plugin_suite, fixture)

BOOST_AUTO_TEST_CASE(handle_command)
{
    plugin_->handle_command(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["command"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_connect)
{
    plugin_->handle_connect(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["connect"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_invite)
{
    plugin_->handle_invite(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["invite"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_join)
{
    plugin_->handle_join(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["join"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_kick)
{
    plugin_->handle_kick(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["kick"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_load)
{
    plugin_->handle_load(irccd_);

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["load"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_message)
{
    plugin_->handle_message(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["message"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_me)
{
    plugin_->handle_me(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["me"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_mode)
{
    plugin_->handle_mode(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["mode"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_names)
{
    plugin_->handle_names(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["names"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_nick)
{
    plugin_->handle_nick(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["nick"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_notice)
{
    plugin_->handle_notice(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["notice"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_part)
{
    plugin_->handle_part(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["part"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_reload)
{
    plugin_->handle_reload(irccd_);

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["reload"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_topic)
{
    plugin_->handle_topic(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["topic"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_unload)
{
    plugin_->handle_unload(irccd_);

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["unload"] == "true");
}

BOOST_AUTO_TEST_CASE(handle_whois)
{
    plugin_->handle_whois(irccd_, {});

    BOOST_TEST(plugin_->get_config().size() == 1U);
    BOOST_TEST(plugin_->get_config()["whois"] == "true");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
