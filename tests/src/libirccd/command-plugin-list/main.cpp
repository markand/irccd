/*
 * main.cpp -- test plugin-list remote command
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

#define BOOST_TEST_MODULE "plugin-list"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/command/plugin_list_command.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class sample_plugin : public plugin {
public:
    auto get_name() const noexcept -> std::string_view override
    {
        return "sample";
    }
};

class plugin_list_test : public command_test<plugin_list_command> {
public:
    plugin_list_test()
    {
        daemon_->plugins().add("t1", std::make_unique<sample_plugin>());
        daemon_->plugins().add("t2", std::make_unique<sample_plugin>());
    }
};

BOOST_FIXTURE_TEST_SUITE(plugin_list_test_suite, plugin_list_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto [ result, code ] = request({
        { "command", "plugin-list" }
    });

    BOOST_TEST(!code);
    BOOST_TEST(result.is_object());
    BOOST_TEST(result["list"][0].template get<std::string>() == "t1");
    BOOST_TEST(result["list"][1].template get<std::string>() == "t2");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
