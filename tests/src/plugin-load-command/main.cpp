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

            void on_load(irccd&) override
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

BOOST_AUTO_TEST_CASE(not_found)
{
    boost::system::error_code result;

    ctl_->send({
        { "command",    "plugin-load"   },
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

BOOST_AUTO_TEST_CASE(already_exists)
{
    boost::system::error_code result;

    ctl_->send({
        { "command",    "plugin-load"   },
        { "plugin",     "already"       }
    });
    ctl_->recv([&] (auto code, auto) {
        result = code;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == plugin_error::already_exists);
}

BOOST_AUTO_TEST_CASE(exec_error)
{
    boost::system::error_code result;

    ctl_->send({
        { "command",    "plugin-load"   },
        { "plugin",     "broken"        }
    });
    ctl_->recv([&] (auto code, auto) {
        result = code;
    });

    wait_for([&] {
        return result;
    });

    BOOST_ASSERT(result == plugin_error::exec_error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
