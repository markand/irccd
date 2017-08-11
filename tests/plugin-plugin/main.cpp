/*
 * main.cpp -- test plugin plugin
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

#include <gtest/gtest.h>

#include <format.h>

#include <irccd/irccd.hpp>
#include <irccd/logger.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

using namespace fmt::literals;

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

class fake_plugin : public plugin {
public:
    fake_plugin()
        : plugin("fake", "")
    {
        set_author("jean");
        set_version("0.0.0.0.0.1");
        set_license("BEER");
        set_summary("Fake White Beer 2000");
    }
};

class plugin_test_suite : public plugin_test {
protected:
    std::shared_ptr<server_test> server_;

public:
    plugin_test_suite()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
        , server_(std::make_shared<server_test>())
    {
        irccd_.plugins().add(std::make_shared<fake_plugin>());

        plugin_->set_formats({
            { "usage", "usage=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" },
            { "info", "info=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{author}:#{license}:#{name}:#{summary}:#{version}" },
            { "not-found", "not-found=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}:#{name}" },
            { "too-long", "too-long=#{plugin}:#{command}:#{server}:#{channel}:#{origin}:#{nickname}" }
        });
        plugin_->on_load(irccd_);
    }
};

TEST_F(plugin_test_suite, formatUsage)
{
    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", ""});
    ASSERT_EQ("#staff:usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean", server_->last());

    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", "fail"});
    ASSERT_EQ("#staff:usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean", server_->last());

    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", "info"});
    ASSERT_EQ("#staff:usage=plugin:!plugin:test:#staff:jean!jean@localhost:jean", server_->last());
}

TEST_F(plugin_test_suite, formatInfo)
{
    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", "info fake"});

    ASSERT_EQ("#staff:info=plugin:!plugin:test:#staff:jean!jean@localhost:jean:jean:BEER:fake:Fake White Beer 2000:0.0.0.0.0.1", server_->last());
}

TEST_F(plugin_test_suite, formatNotFound)
{
    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", "info doesnotexistsihope"});

    ASSERT_EQ("#staff:not-found=plugin:!plugin:test:#staff:jean!jean@localhost:jean:doesnotexistsihope", server_->last());
}

TEST_F(plugin_test_suite, formatTooLong)
{
    for (int i = 0; i < 100; ++i)
        irccd_.plugins().add(std::make_shared<plugin>("plugin-n-{}"_format(i), ""));

    plugin_->on_command(irccd_, MessageEvent{server_, "jean!jean@localhost", "#staff", "list"});

    ASSERT_EQ("#staff:too-long=plugin:!plugin:test:#staff:jean!jean@localhost:jean", server_->last());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    log::setLogger(std::make_unique<log::SilentLogger>());

    return RUN_ALL_TESTS();
}
