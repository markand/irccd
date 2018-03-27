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

#include <irccd/daemon/command/plugin_load_command.hpp>
#include <irccd/daemon/service/plugin_service.hpp>

#include <irccd/test/command_test.hpp>

namespace irccd {

namespace {

class custom_loader : public plugin_loader {
public:
    custom_loader()
        : plugin_loader({}, {".none"})
    {
    }

    std::shared_ptr<plugin> open(const std::string&,
                                 const std::string&) noexcept override
    {
        return nullptr;
    }

    std::shared_ptr<plugin> find(const std::string& id) noexcept override
    {
        class broken : public plugin {
        public:
            using plugin::plugin;

            void handle_load(irccd&) override
            {
                throw std::runtime_error("broken");
            }
        };

        /*
         * The 'magic' plugin will be created for the unit tests, all other
         * plugins will return null.
         */
        if (id == "magic")
            return std::make_unique<plugin>(id, "");
        if (id == "broken")
            return std::make_unique<broken>(id, "");

        return nullptr;
    }
};

class plugin_load_test : public command_test<plugin_load_command> {
public:
    plugin_load_test()
    {
        daemon_->plugins().add_loader(std::make_unique<custom_loader>());
        daemon_->plugins().add(std::make_unique<plugin>("already", ""));
    }
};

} // !namespace

BOOST_FIXTURE_TEST_SUITE(plugin_load_test_suite, plugin_load_test)

BOOST_AUTO_TEST_CASE(basic)
{
    ctl_->send({
        { "command",    "plugin-load"   },
        { "plugin",     "magic"         }
    });

    wait_for([&] () {
        return daemon_->plugins().has("magic");
    });

    BOOST_TEST(!daemon_->plugins().list().empty());
    BOOST_TEST(daemon_->plugins().has("magic"));
}

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_identifier)
{
    const auto result = request({
        { "command",    "plugin-load"   }
    });

    BOOST_TEST(result.second == plugin_error::invalid_identifier);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::invalid_identifier);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(not_found)
{
    const auto result = request({
        { "command",    "plugin-load"   },
        { "plugin",     "unknown"       }
    });

    BOOST_TEST(result.second == plugin_error::not_found);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::not_found);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(already_exists)
{
    const auto result = request({
        { "command",    "plugin-load"   },
        { "plugin",     "already"       }
    });

    BOOST_TEST(result.second == plugin_error::already_exists);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::already_exists);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_CASE(exec_error)
{
    const auto result = request({
        { "command",    "plugin-load"   },
        { "plugin",     "broken"        }
    });

    BOOST_TEST(result.second == plugin_error::exec_error);
    BOOST_TEST(result.first["error"].template get<int>() == plugin_error::exec_error);
    BOOST_TEST(result.first["errorCategory"].template get<std::string>() == "plugin");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
