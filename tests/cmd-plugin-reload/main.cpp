/*
 * main.cpp -- test plugin-reload remote command
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

#define BOOST_TEST_MODULE "plugin-reload"
#include <boost/test/unit_test.hpp>

#include <irccd/command.hpp>
#include <irccd/plugin_service.hpp>

#include <command_test.hpp>

namespace irccd {

namespace {

class custom_plugin : public plugin {
public:
    bool reloaded{false};

    custom_plugin()
        : plugin("test", "")
    {
    }

    void on_reload(irccd&) override
    {
        reloaded = true;
    }
};

class plugin_reload_test : public command_test<plugin_reload_command> {
protected:
    std::shared_ptr<custom_plugin> plugin_;

    plugin_reload_test()
        : plugin_(std::make_shared<custom_plugin>())
    {
        daemon_->plugins().add(plugin_);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_reload_test_suite, plugin_reload_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "plugin-reload" },
        { "plugin",     "test"          }
    });

    wait_for([&] () {
        return plugin_->reloaded;
    });

    BOOST_TEST(plugin_->reloaded);
}

BOOST_AUTO_TEST_CASE(notfound)
{
    auto response = nlohmann::json();

    ctl_->recv([&] (auto, auto msg) {
        response = msg;
    });
    ctl_->send({
        { "command",    "plugin-reload" },
        { "plugin",     "no"            }
    });

    wait_for([&] () {
        return response.is_object();
    });

    // TODO: error code
    BOOST_TEST(response.is_object());
    BOOST_TEST(response["error"].get<std::string>() == "plugin no not found");
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
