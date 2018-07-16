/*
 * main.cpp -- test irccdctl plugin-info
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

#define BOOST_TEST_MODULE "irccdctl plugin-info"
#include <boost/test/unit_test.hpp>

#include <irccd/test/plugin_cli_test.hpp>

namespace irccd {

namespace {

class sample : public plugin {
public:
    auto get_name() const noexcept -> std::string_view override
    {
        return "sample";
    }

    auto get_author() const noexcept -> std::string_view override
    {
        return "David Demelier <markand@malikania.fr>";
    }

    auto get_license() const noexcept -> std::string_view override
    {
        return "ISC";
    }

    auto get_summary() const noexcept -> std::string_view override
    {
        return "foo";
    }

    auto get_version() const noexcept -> std::string_view override
    {
        return "0.0";
    }
};

BOOST_FIXTURE_TEST_SUITE(plugin_info_suite, plugin_cli_test)

BOOST_AUTO_TEST_CASE(simple)
{
    auto p = std::make_unique<sample>();

    irccd_.plugins().add("p", std::move(p));
    start();

    const auto result = exec({ "plugin-info", "p" });

    BOOST_TEST(result.first.size() == 4U);
    BOOST_TEST(result.second.size() == 0U);
    BOOST_TEST(result.first[0] == "Author         : David Demelier <markand@malikania.fr>");
    BOOST_TEST(result.first[1] == "License        : ISC");
    BOOST_TEST(result.first[2] == "Summary        : foo");
    BOOST_TEST(result.first[3] == "Version        : 0.0");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
