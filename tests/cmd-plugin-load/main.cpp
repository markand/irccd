/*
 * main.cpp -- test plugin-load remote command
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

#define BOOST_TEST_MODULE "plugin-load"
#include <boost/test/unit_test.hpp>

#include <irccd/command.hpp>
#include <irccd/plugin_service.hpp>

#include <command_test.hpp>

namespace irccd {

namespace {

class custom_loader : public plugin_loader {
public:
    std::shared_ptr<plugin> open(const std::string&,
                                 const std::string&) noexcept override
    {
        return nullptr;
    }

    std::shared_ptr<plugin> find(const std::string& id) noexcept override
    {
        return std::make_unique<plugin>(id, "");
    }
};

class plugin_load_test : public command_test<plugin_load_command> {
public:
    plugin_load_test()
    {
        daemon_->plugins().add_loader(std::make_unique<custom_loader>());
    }
};

} // !irccd

BOOST_FIXTURE_TEST_SUITE(plugin_load_test_suite, plugin_load_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command", "plugin-load" },
        { "plugin", "foo" }
    });

    wait_for([&] () {
        return daemon_->plugins().has("foo");
    });

    BOOST_TEST(!daemon_->plugins().list().empty());
    BOOST_TEST(daemon_->plugins().has("foo"));
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
