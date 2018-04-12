/*
 * main.cpp -- test irccdctl plugin-load
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

#define BOOST_TEST_MODULE "irccdctl plugin-load"
#include <boost/test/unit_test.hpp>

#include <irccd/test/plugin_cli_test.hpp>

namespace irccd {

namespace {

class custom_plugin_loader : public plugin_loader {
public:
    custom_plugin_loader()
        : plugin_loader({}, { "none" })
    {
    }

    std::shared_ptr<plugin> find(const std::string& id) override
    {
        return std::make_unique<plugin>(id, "local");
    }

    std::shared_ptr<plugin> open(const std::string& id, const std::string& path) override
    {
        return std::make_unique<plugin>(id, path);
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_load_suite, plugin_cli_test)

BOOST_AUTO_TEST_CASE(simple)
{
    irccd_.plugins().add(std::make_unique<plugin>("p1", "local"));
    irccd_.plugins().add(std::make_unique<plugin>("p2", "local"));
    irccd_.plugins().add_loader(std::make_unique<custom_plugin_loader>());
    start();

    // Load a plugin first.
    {
        const auto result = exec({ "plugin-load", "test-cli-plugin-load" });

        BOOST_TEST(result.first.size() == 0U);
        BOOST_TEST(result.second.size() == 0U);
    }

    // Get the new list of plugins.
    {
        const auto result = exec({ "plugin-list" });

        BOOST_TEST(result.first.size() == 3U);
        BOOST_TEST(result.second.size() == 0U);
        BOOST_TEST(result.first[0] == "p1");
        BOOST_TEST(result.first[1] == "p2");
        BOOST_TEST(result.first[2] == "test-cli-plugin-load");
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
