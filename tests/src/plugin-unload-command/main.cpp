/*
 * main.cpp -- test plugin-unload remote command
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

#define BOOST_TEST_MODULE "plugin-unload"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include <command_test.hpp>

namespace irccd {

namespace {

class custom_plugin : public plugin {
public:
    bool unloaded{false};

    custom_plugin()
        : plugin("test", "")
    {
    }

    void on_unload(irccd &) override
    {
        unloaded = true;
    }
};

class broken_plugin : public plugin {
public:
    using plugin::plugin;

    void on_unload(irccd&) override
    {
        throw std::runtime_error("broken");
    }
};

class plugin_unload_test : public command_test<plugin_unload_command> {
protected:
    std::shared_ptr<custom_plugin> plugin_;

    plugin_unload_test()
        : plugin_(std::make_shared<custom_plugin>())
    {
        daemon_->plugins().add(plugin_);
        daemon_->plugins().add(std::make_unique<broken_plugin>("broken", ""));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_unload_test_suite, plugin_unload_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "plugin-unload" },
        { "plugin",     "test"          }
    });

    wait_for([&] () {
        return plugin_->unloaded;
    });

    BOOST_TEST(plugin_->unloaded);
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;

    ctl_->send({
        { "command",    "plugin-unload" },
        { "plugin",     "unknown"       }
    });
    ctl_->recv([&] (auto code, auto) {
        result = code;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == plugin_error::not_found);
}

BOOST_AUTO_TEST_CASE(exec_error)
{
    boost::system::error_code result;

    ctl_->send({
        { "command",    "plugin-unload" },
        { "plugin",     "broken"        }
    });
    ctl_->recv([&] (auto code, auto) {
        result = code;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == plugin_error::exec_error);
    BOOST_ASSERT(!daemon_->plugins().has("broken"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
