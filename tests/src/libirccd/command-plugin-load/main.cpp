/*
 * main.cpp -- test plugin-load remote command
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

#define BOOST_TEST_MODULE "plugin-load"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/plugin_service.hpp>

#include <irccd/test/command_test.hpp>
#include <irccd/test/mock_plugin.hpp>

namespace irccd {

namespace {

class broken : public plugin {
public:
    broken()
        : plugin("broken")
    {
    }

    auto get_name() const noexcept -> std::string_view override
    {
        return "broken";
    }

    void handle_load(irccd&) override
    {
        throw std::runtime_error("broken");
    }
};

class broken_loader : public plugin_loader {
public:
    broken_loader()
        : plugin_loader({}, { ".none" })
    {
    }

    auto open(std::string_view, std::string_view) -> std::shared_ptr<plugin> override
    {
        return nullptr;
    }

    auto find(std::string_view id) noexcept -> std::shared_ptr<plugin> override
    {
        if (id == "broken")
            return std::make_unique<broken>();

        return nullptr;
    }
};

class sample_loader : public plugin_loader {
public:
    sample_loader()
        : plugin_loader({}, { ".none" })
    {
    }

    auto open(std::string_view, std::string_view) -> std::shared_ptr<plugin> override
    {
        return nullptr;
    }

    auto find(std::string_view id) noexcept -> std::shared_ptr<plugin> override
    {
        if (id == "test")
            return std::make_unique<mock_plugin>("test");

        return nullptr;
    }
};

class plugin_load_test : public command_test<plugin_load_command> {
public:
    plugin_load_test()
    {
        daemon_->plugins().add_loader(std::make_unique<sample_loader>());
        daemon_->plugins().add_loader(std::make_unique<broken_loader>());
        daemon_->plugins().add(std::make_unique<mock_plugin>("already"));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_load_test_suite, plugin_load_test)

BOOST_AUTO_TEST_CASE(basic)
{
    const auto [json, code] = request({
        { "command",    "plugin-load"   },
        { "plugin",     "test"          }
    });

    BOOST_TEST(!code);
    BOOST_TEST(daemon_->plugins().has("test"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    const auto [json, code] = request({
        { "command",    "plugin-load"   }
    });

    BOOST_TEST(code == plugin_error::invalid_identifier);
    BOOST_TEST(json["error"].get<int>() == plugin_error::invalid_identifier);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto [json, code] = request({
        { "command",    "plugin-load"   },
        { "plugin",     "unknown"       }
    });

    BOOST_TEST(code == plugin_error::not_found);
    BOOST_TEST(json["error"].get<int>() == plugin_error::not_found);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(already_exists)
{
    const auto [json, code] = request({
        { "command",    "plugin-load"   },
        { "plugin",     "already"       }
    });

    BOOST_TEST(code == plugin_error::already_exists);
    BOOST_TEST(json["error"].get<int>() == plugin_error::already_exists);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(exec_error)
{
    const auto [json, code] = request({
        { "command",    "plugin-load"   },
        { "plugin",     "broken"        }
    });

    BOOST_TEST(code == plugin_error::exec_error);
    BOOST_TEST(json["error"].get<int>() == plugin_error::exec_error);
    BOOST_TEST(json["errorCategory"].get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
