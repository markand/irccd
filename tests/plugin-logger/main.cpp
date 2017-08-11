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

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

using namespace irccd;

class logger_test : public plugin_test {
protected:
    std::shared_ptr<Server> server_;

    std::string last() const
    {
        std::ifstream file(BINARYDIR "/log.txt");

        return std::string(std::istreambuf_iterator<char>(file.rdbuf()), {});
    }

public:
    logger_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
        , server_(std::make_shared<Server>("test"))
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

TEST_F(logger_test, formatChannelMode)
{
    load();

    plugin_->on_channel_mode(irccd_, ChannelModeEvent{server_, "jean!jean@localhost", "#staff", "+o", "jean"});

    ASSERT_EQ("cmode=test:#staff:jean!jean@localhost:jean:+o:jean\n", last());
}

TEST_F(logger_test, formatChannelNotice)
{
    load();

    plugin_->on_channel_notice(irccd_, ChannelNoticeEvent{server_, "jean!jean@localhost", "#staff", "bonjour!"});

    ASSERT_EQ("cnotice=test:#staff:jean!jean@localhost:jean:bonjour!\n", last());
}

TEST_F(logger_test, formatJoin)
{
    load();

    plugin_->on_join(irccd_, JoinEvent{server_, "jean!jean@localhost", "#staff"});

    ASSERT_EQ("join=test:#staff:jean!jean@localhost:jean\n", last());
}

TEST_F(logger_test, formatKick)
{
    load();

    plugin_->on_kick(irccd_, KickEvent{server_, "jean!jean@localhost", "#staff", "badboy", "please do not flood"});

    ASSERT_EQ("kick=test:#staff:jean!jean@localhost:jean:badboy:please do not flood\n", last());
}

TEST_F(logger_test, formatMe)
{
    load();

    plugin_->on_me(irccd_, MeEvent{server_, "jean!jean@localhost", "#staff", "is drinking water"});

    ASSERT_EQ("me=test:#staff:jean!jean@localhost:jean:is drinking water\n", last());
}

TEST_F(logger_test, formatMessage)
{
    load();

    plugin_->on_message(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", "hello guys"});

    ASSERT_EQ("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

TEST_F(logger_test, formatMode)
{
    load();

    plugin_->on_mode(irccd_, ModeEvent{server_, "jean!jean@localhost", "+i"});

    ASSERT_EQ("mode=test:jean!jean@localhost:jean:+i:\n", last());
}

TEST_F(logger_test, formatNotice)
{
    load();

    plugin_->on_notice(irccd_, NoticeEvent{server_, "jean!jean@localhost", "tu veux voir mon chat ?"});

    ASSERT_EQ("notice=test:jean!jean@localhost:jean:tu veux voir mon chat ?\n", last());
}

TEST_F(logger_test, formatPart)
{
    load();

    plugin_->on_part(irccd_, PartEvent{server_, "jean!jean@localhost", "#staff", "too noisy here"});

    ASSERT_EQ("part=test:#staff:jean!jean@localhost:jean:too noisy here\n", last());
}

TEST_F(logger_test, formatQuery)
{
    load();

    plugin_->on_query(irccd_, QueryEvent{server_, "jean!jean@localhost", "much irccd, wow"});

    ASSERT_EQ("query=test:jean!jean@localhost:jean:much irccd, wow\n", last());
}

TEST_F(logger_test, formatTopic)
{
    load();

    plugin_->on_topic(irccd_, TopicEvent{server_, "jean!jean@localhost", "#staff", "oh yeah yeaaaaaaaah"});

    ASSERT_EQ("topic=test:#staff:jean!jean@localhost:jean:oh yeah yeaaaaaaaah\n", last());
}

TEST_F(logger_test, case_fix_642)
{
    load();

    plugin_->on_message(irccd_, MessageEvent{server_, "jean!jean@localhost", "#STAFF", "hello guys"});

    ASSERT_EQ("message=test:#staff:jean!jean@localhost:jean:hello guys\n", last());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    log::setLogger(std::make_unique<log::SilentLogger>());

    return RUN_ALL_TESTS();
}
