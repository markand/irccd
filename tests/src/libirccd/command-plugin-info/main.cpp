/*
 * main.cpp -- test plugin-info remote command
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

#define BOOST_TEST_MODULE "plugin-info"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/plugin_info_command.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

BOOST_FIXTURE_TEST_SUITE(plugin_info_test_suite, command_test<plugin_info_command>)

BOOST_AUTO_TEST_CASE(basic)
{
    auto plg = std::make_unique<plugin>("test", "");
    auto response = nlohmann::json();

    plg->set_author("Francis Beaugrand");
    plg->set_license("GPL");
    plg->set_summary("Completely useless plugin");
    plg->set_version("0.0.0.0.0.0.0.0.1-beta5");

    daemon_->plugins().add(std::move(plg));
    ctl_->recv([&] (auto, auto msg) {
        response = std::move(msg);
    });
    ctl_->send({
        { "command",    "plugin-info"       },
        { "plugin",     "test"              },
    });

    wait_for([&] () {
        return response.is_object();
    });

    BOOST_TEST(response.is_object());
    BOOST_TEST(response["author"].get<std::string>() == "Francis Beaugrand");
    BOOST_TEST(response["license"].get<std::string>() == "GPL");
    BOOST_TEST(response["summary"].get<std::string>() == "Completely useless plugin");
    BOOST_TEST(response["version"].get<std::string>() == "0.0.0.0.0.0.0.0.1-beta5");
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "plugin-info"   }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == plugin_error::invalid_identifier);
    BOOST_TEST(message["error"].template get<int>() == plugin_error::invalid_identifier);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;
    nlohmann::json message;

    ctl_->send({
        { "command",    "plugin-info"   },
        { "plugin",     "unknown"       }
    });
    ctl_->recv([&] (auto rresult, auto rmessage) {
        result = rresult;
        message = rmessage;
    });

    wait_for([&] {
        return result;
    });

    BOOST_TEST(result == plugin_error::not_found);
    BOOST_TEST(message["error"].template get<int>() == plugin_error::not_found);
    BOOST_TEST(message["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
