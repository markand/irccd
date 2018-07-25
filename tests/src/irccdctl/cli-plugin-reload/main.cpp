/*
 * main.cpp -- test irccdctl plugin-reload
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

#define BOOST_TEST_MODULE "irccdctl plugin-reload"
#include <boost/test/unit_test.hpp>

#include <irccd/test/plugin_cli_test.hpp>
#include <irccd/test/mock.hpp>

namespace irccd {

namespace {

class reloadable_plugin : public mock, public plugin {
public:
    reloadable_plugin()
        : plugin("test")
    {
    }

    auto get_name() const noexcept -> std::string_view override
    {
        return "reload";
    }

    void handle_reload(irccd&) override
    {
        push("handle_reload");
    }
};

BOOST_FIXTURE_TEST_SUITE(plugin_reload_suite, plugin_cli_test)

BOOST_AUTO_TEST_CASE(simple)
{
    const auto plugin = std::make_shared<reloadable_plugin>();

    irccd_.plugins().add(plugin);
    start();

    const auto [code, out, err] = exec({ "plugin-reload", "test" });

    BOOST_TEST(!code);
    BOOST_TEST(out.size() == 0U);
    BOOST_TEST(err.size() == 0U);
    BOOST_TEST(plugin->find("handle_reload").size() == 1U);
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
