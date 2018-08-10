/*
 * main.cpp -- test js_plugin object
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

#define BOOST_TEST_MODULE "Javascript plugin object"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/plugin_service.hpp>

#include <irccd/js/js_api.hpp>
#include <irccd/js/js_plugin.hpp>

#include <irccd/test/irccd_fixture.hpp>

using namespace irccd::test;

namespace irccd {

namespace {

class js_plugin_fixture : public irccd_fixture {
protected:
    std::shared_ptr<js::js_plugin> plugin_;

    void load(const std::string& path)
    {
        plugin_ = std::make_unique<js::js_plugin>("test", path);

        for (const auto& f : js::js_api::registry)
            f()->load(irccd_, plugin_);

        plugin_->open();
    }
};

BOOST_FIXTURE_TEST_SUITE(js_plugin_suite, js_plugin_fixture)

BOOST_AUTO_TEST_CASE(assign)
{
    load(CMAKE_CURRENT_SOURCE_DIR "/config-assign.js");

    plugin_->set_options({
        { "path",       "none"  },
        { "verbose",    "false" }
    });
    plugin_->handle_load(irccd_);

    BOOST_TEST(plugin_->get_options().at("path") == "none");
    BOOST_TEST(plugin_->get_options().at("verbose") == "false");
    BOOST_TEST(plugin_->get_options().at("hard") == "true");
}

BOOST_AUTO_TEST_CASE(fill)
{
    load(CMAKE_CURRENT_SOURCE_DIR "/config-fill.js");

    plugin_->set_options({
        { "path",       "none"  },
        { "verbose",    "false" }
    });
    plugin_->handle_load(irccd_);

    BOOST_TEST(plugin_->get_options().at("path") == "none");
    BOOST_TEST(plugin_->get_options().at("verbose") == "false");
    BOOST_TEST(plugin_->get_options().at("hard") == "true");
}

BOOST_AUTO_TEST_CASE(merge_after)
{
    load(CMAKE_CURRENT_SOURCE_DIR "/config-fill.js");

    plugin_->handle_load(irccd_);
    plugin_->set_options({
        { "path",       "none"  },
        { "verbose",    "false" }
    });

    BOOST_TEST(plugin_->get_options().at("path") == "none");
    BOOST_TEST(plugin_->get_options().at("verbose") == "false");
    BOOST_TEST(plugin_->get_options().at("hard") == "true");
}

BOOST_AUTO_TEST_SUITE_END()

class js_plugin_loader_fixture : public irccd_fixture {
protected:
    std::shared_ptr<plugin> plugin_;

    js_plugin_loader_fixture()
    {
        irccd_.set_config(config(CMAKE_CURRENT_SOURCE_DIR "/irccd.conf"));

        auto loader = std::make_unique<js::js_plugin_loader>(irccd_);

        for (const auto& f : js::js_api::registry)
            loader->get_modules().push_back(f());

        irccd_.plugins().add_loader(std::move(loader));
    }

    void load(std::string name, std::string path)
    {
        irccd_.plugins().load(name, path);
        plugin_ = irccd_.plugins().require(name);
    }
};

BOOST_FIXTURE_TEST_SUITE(js_plugin_loader_test_suite, js_plugin_loader_fixture)

BOOST_AUTO_TEST_CASE(assign)
{
    load("test", CMAKE_CURRENT_SOURCE_DIR "/config-assign.js");

    BOOST_TEST(plugin_->get_options().at("path") == "none");
    BOOST_TEST(plugin_->get_options().at("verbose") == "false");
    BOOST_TEST(plugin_->get_options().at("hard") == "true");
}

BOOST_AUTO_TEST_CASE(fill)
{
    load("test", CMAKE_CURRENT_SOURCE_DIR "/config-fill.js");

    BOOST_TEST(plugin_->get_options().at("path") == "none");
    BOOST_TEST(plugin_->get_options().at("verbose") == "false");
    BOOST_TEST(plugin_->get_options().at("hard") == "true");
}

BOOST_AUTO_TEST_CASE(merge_after)
{
    load("test", CMAKE_CURRENT_SOURCE_DIR "/config-fill.js");

    BOOST_TEST(plugin_->get_options().at("path") == "none");
    BOOST_TEST(plugin_->get_options().at("verbose") == "false");
    BOOST_TEST(plugin_->get_options().at("hard") == "true");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
