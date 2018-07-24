/*
 * main.cpp -- test irccdctl plugin-config
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

#define BOOST_TEST_MODULE "irccdctl plugin-config"
#include <boost/test/unit_test.hpp>

#include <irccd/test/plugin_cli_test.hpp>

namespace irccd {

namespace {

class configurable_plugin : public plugin {
private:
    map config_;

public:
    using plugin::plugin;

    auto get_name() const noexcept -> std::string_view override
    {
        return "config";
    }

    auto get_options() const -> map override
    {
        return config_;
    }

    void set_options(const map& config) override
    {
        config_ = std::move(config);
    }
};

class configurable_plugin_cli_test : public plugin_cli_test {
public:
    configurable_plugin_cli_test()
    {
        auto conf1 = std::make_unique<configurable_plugin>("conf1");
        auto conf2 = std::make_unique<configurable_plugin>("conf2");

        conf1->set_options({
            { "v1", "123" },
            { "v2", "456" }
        });

        irccd_.plugins().add(std::move(conf1));
        irccd_.plugins().add(std::move(conf2));
    }
};

BOOST_FIXTURE_TEST_SUITE(plugin_config_suite, configurable_plugin_cli_test)

BOOST_AUTO_TEST_CASE(set_and_get)
{
    start();

    // First, configure. No output yet
    {
        const auto [out, err] = exec({ "plugin-config", "conf2", "verbose", "false" });

        // no output yet.
        BOOST_TEST(out.size() == 0U);
        BOOST_TEST(err.size() == 0U);
    }

    // Get the newly created value.
    {
        const auto [out, err] = exec({ "plugin-config", "conf2", "verbose" });

        BOOST_TEST(out.size() == 1U);
        BOOST_TEST(err.size() == 0U);
        BOOST_TEST(out[0] == "false");
    }
}

BOOST_AUTO_TEST_CASE(getall)
{
    start();

    const auto [out, err] = exec({ "plugin-config", "conf1" });

    BOOST_TEST(out.size() == 2U);
    BOOST_TEST(err.size() == 0U);
    BOOST_TEST(out[0] == "v1               : 123");
    BOOST_TEST(out[1] == "v2               : 456");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
