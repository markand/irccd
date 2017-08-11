/*
 * main.cpp -- test history plugin
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

#include <regex>

#include <gtest/gtest.h>

#include <irccd/irccd.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

using namespace irccd;

class server_test : public Server {
private:
    std::string last_;

public:
    inline server_test()
        : Server("test")
    {
    }

    inline const std::string& last() const noexcept
    {
        return last_;
    }

    void message(std::string target, std::string message) override
    {
        last_ = util::join({target, message});
    }
};

class history_test : public plugin_test {
protected:
    std::shared_ptr<server_test> server_;

public:
    history_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
        , server_(std::make_shared<server_test>())
    {
        plugin_->set_formats({
            { "error", "error=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
            { "seen", "seen=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:%H:%M" },
            { "said", "said=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}:#{message}:%H:%M" },
            { "unknown", "unknown=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{target}" },
        });
    }

    void load(plugin_config config = {})
    {
        // Add file if not there.
        if (config.count("file") == 0)
            config.emplace("file", SOURCEDIR "/words.conf");

        plugin_->set_config(config);
        plugin_->on_load(irccd_);
    }
};

TEST_F(history_test, formatError)
{
    load({{ "file", SOURCEDIR "/broken-conf.json" }});

    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#history", "seen francis"});
    ASSERT_EQ("#history:error=history:!history:test:#history:jean!jean@localhost:jean", server_->last());
}

TEST_F(history_test, formatSeen)
{
    std::regex rule("#history:seen=history:!history:test:#history:destructor!dst@localhost:destructor:jean:\\d{2}:\\d{2}");

    remove(BINARYDIR "/seen.json");
    load({{ "file", BINARYDIR "/seen.json" }});

    plugin_->on_message(irccd_, MessageEvent{server_, "jean!jean@localhost", "#history", "hello"});
    plugin_->on_command(irccd_, MessageEvent{server_, "destructor!dst@localhost", "#history", "seen jean"});

    ASSERT_TRUE(std::regex_match(server_->last(), rule));
}

TEST_F(history_test, formatSaid)
{
    std::regex rule("#history:said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

    remove(BINARYDIR "/said.json");
    load({{ "file", BINARYDIR "/said.json" }});

    plugin_->on_message(irccd_, MessageEvent{server_, "jean!jean@localhost", "#history", "hello"});
    plugin_->on_command(irccd_, MessageEvent{server_, "destructor!dst@localhost", "#history", "said jean"});

    ASSERT_TRUE(std::regex_match(server_->last(), rule));
}

TEST_F(history_test, formatUnknown)
{
    remove(BINARYDIR "/unknown.json");
    load({{ "file", BINARYDIR "/unknown.json" }});

    plugin_->on_message(irccd_, MessageEvent{server_, "jean!jean@localhost", "#history", "hello"});
    plugin_->on_command(irccd_, MessageEvent{server_, "destructor!dst@localhost", "#history", "seen nobody"});

    ASSERT_EQ("#history:unknown=history:!history:test:#history:destructor!dst@localhost:destructor:nobody", server_->last());
}

TEST_F(history_test, case_fix_642)
{
    std::regex rule("#history:said=history:!history:test:#history:destructor!dst@localhost:destructor:jean:hello:\\d{2}:\\d{2}");

    remove(BINARYDIR "/case.json");
    load({{"file", BINARYDIR "/case.json"}});

    plugin_->on_message(irccd_, MessageEvent{server_, "JeaN!JeaN@localhost", "#history", "hello"});

    plugin_->on_command(irccd_, MessageEvent{server_, "destructor!dst@localhost", "#HISTORY", "said JEAN"});
    ASSERT_TRUE(std::regex_match(server_->last(), rule));
    plugin_->on_command(irccd_, MessageEvent{server_, "destructor!dst@localhost", "#HiSToRy", "said JeaN"});
    ASSERT_TRUE(std::regex_match(server_->last(), rule));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
