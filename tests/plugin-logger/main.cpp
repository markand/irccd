/*
 * main.cpp -- test logger plugin
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

#include <fstream>
#include <iterator>

#define BOOST_TEST_MODULE "Logger plugin"
#include <boost/test/unit_test.hpp>

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

namespace irccd {

class logger_test : public plugin_test {
protected:
    std::string last() const
    {
        std::ifstream file(BINARYDIR "/log.txt");

        return std::string(std::istreambuf_iterator<char>(file.rdbuf()), {});
    }

public:
    logger_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        remove(BINARYDIR "/log.txt");

        plugin_->set_formats({
            { "cmode", "cmode=#{server}:#{channel}:#{origin}:#{nickname}:#{mode}:#{arg}" },
            { "cnotice", "cnotice=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
            { "join", "join=#{server}:#{channel}:#{origin}:#{nickname}" },
            { "kick", "kick=#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{reason}" },
            { "me", "me=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
            { "message", "message=#{server}:#{channel}:#{origin}:#{nickname}:#{message}" },
            { "mode", "mode=#{server}:#{origin}:#{nickname}:#{mode}:#{arg}" },
            { "notice", "notice=#{server}:#{origin}:#{nickname}:#{message}" },
            { "part", "part=#{server}:#{channel}:#{origin}:#{nickname}:#{reason}" },
            { "query", "query=#{server}:#{origin}:#{nickname}:#{message}" },
            { "topic", "topic=#{server}:#{channel}:#{origin}:#{nickname}:#{topic}" },
        });
    }

    void load(plugin_config config = plugin_config())
    {
        if (config.count("path") == 0)
            config.emplace("path", BINARYDIR "/log.txt");

        plugin_->set_config(config);
        plugin_->on_load(irccd_);
    }
};

BOOST_FIXTURE_TEST_SUITE(logger_test_suite, logger_test)

BOOST_AUTO_TEST_CASE(format_channel_mode)
{
    load();

    plugin_->on_channel_mode(irccd_, {server_, "jean!jean@localhost", "#staff", "+o", "jean"});

    BOOST_REQUIRE_EQUAL("cmode=test:#staff:jean!jean@localhost:jean:+o:jean\n", last());
}

BOOST_AUTO_TEST_CASE(format_channel_notice)
{
    load();

    plugin_->on_channel_notice(irccd_, {server_, "jean!jean@localhost", "#staff", "bonjour!"});

    BOOST_REQUIRE_EQUAL("cnotice=test:#staff:jean!jean@localhost:jean:bonjour!\n", last());
}

BOOST_AUTO_TEST_CASE(format_join)
{
    load();

    plugin_->on_join(irccd_, {server_, "jean!jean@localhost", "#staff"});

    BOOST_REQUIRE_EQUAL("join=test:#staff:jean!jean@localhost:jean\n", last());
}

BOOST_AUTO_TEST_CASE(format_kick)
{
    load();

    plugin_->on_kick(irccd_, {server_, "jean!jean@localhost", "#staff", "badboy", "please do not flood"});

    BOOST_REQUIRE_EQUAL("kick=test:#staff:jean!jean@localhost:jean:badboy:please do not flood\n", last());
}

BOOST_AUTO_TEST_CASE(format_me)
{
    load();

    plugin_->on_me(irccd_, {server_, "jean!jean@localhost", "#staff", "is drinking water"});

    BOOST_REQUIRE_EQUAL("me=test:#staff:jean!jean@localhost:jean:is drinking water\n", last());
}

BOOST_AUTO_TEST_CASE(format_message)
{
    load();

    plugin_->on_message(irccd_, {server_, "jean!jean@localhost", "#staff", "hello guys"});

    BOOST_REQUIRE_EQUAL("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

BOOST_AUTO_TEST_CASE(format_mode)
{
    load();

    plugin_->on_mode(irccd_, {server_, "jean!jean@localhost", "+i"});

    BOOST_REQUIRE_EQUAL("mode=test:jean!jean@localhost:jean:+i:\n", last());
}

BOOST_AUTO_TEST_CASE(format_notice)
{
    load();

    plugin_->on_notice(irccd_, {server_, "jean!jean@localhost", "tu veux voir mon chat ?"});

    BOOST_REQUIRE_EQUAL("notice=test:jean!jean@localhost:jean:tu veux voir mon chat ?\n", last());
}

BOOST_AUTO_TEST_CASE(format_part)
{
    load();

    plugin_->on_part(irccd_, {server_, "jean!jean@localhost", "#staff", "too noisy here"});

    BOOST_REQUIRE_EQUAL("part=test:#staff:jean!jean@localhost:jean:too noisy here\n", last());
}

BOOST_AUTO_TEST_CASE(format_query)
{
    load();

    plugin_->on_query(irccd_, {server_, "jean!jean@localhost", "much irccd, wow"});

    BOOST_REQUIRE_EQUAL("query=test:jean!jean@localhost:jean:much irccd, wow\n", last());
}

BOOST_AUTO_TEST_CASE(format_topic)
{
    load();

    plugin_->on_topic(irccd_, {server_, "jean!jean@localhost", "#staff", "oh yeah yeaaaaaaaah"});

    BOOST_REQUIRE_EQUAL("topic=test:#staff:jean!jean@localhost:jean:oh yeah yeaaaaaaaah\n", last());
}

BOOST_AUTO_TEST_CASE(fix_642)
{
    load();

    plugin_->on_message(irccd_, {server_, "jean!jean@localhost", "#STAFF", "hello guys"});

    BOOST_REQUIRE_EQUAL("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
