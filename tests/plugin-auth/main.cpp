/*
 * main.cpp -- test auth plugin
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <irccd/irccd.hpp>
#include <irccd/server.hpp>
#include <irccd/service-plugin.hpp>

using namespace irccd;

class ServerTest : public Server {
private:
    std::string m_last;

public:
    inline ServerTest(std::string name)
        : Server(std::move(name), ServerInfo())
    {
    }

    inline const std::string &last() const noexcept
    {
        return m_last;
    }

    void message(std::string target, std::string message) override
    {
        m_last = util::join({target, message});
    }
};

class AuthTest : public testing::Test {
protected:
    Irccd m_irccd;
    PluginService &m_ps;

    std::shared_ptr<ServerTest> m_nickserv1;
    std::shared_ptr<ServerTest> m_nickserv2;
    std::shared_ptr<ServerTest> m_quakenet;
    std::shared_ptr<Plugin> m_plugin;

public:
    AuthTest()
        : m_ps(m_irccd.pluginService())
        , m_nickserv1(std::make_shared<ServerTest>("nickserv1"))
        , m_nickserv2(std::make_shared<ServerTest>("nickserv2"))
        , m_quakenet(std::make_shared<ServerTest>("quakenet"))
    {
        m_ps.setConfig("auth", {
            { "nickserv1.type", "nickserv" },
            { "nickserv1.password", "plopation" },
            { "nickserv2.type", "nickserv" },
            { "nickserv2.password", "something" },
            { "nickserv2.username", "jean" },
            { "quakenet.type", "quakenet" },
            { "quakenet.password", "hello" },
            { "quakenet.username", "mario" }
        });
        m_ps.load("auth", PLUGINDIR "/auth.js");
        m_plugin = m_ps.require("auth");
    }
};

TEST_F(AuthTest, nickserv1)
{
    m_plugin->onConnect(m_irccd, ConnectEvent{m_nickserv1});

    ASSERT_EQ("NickServ:identify plopation", m_nickserv1->last());
}

TEST_F(AuthTest, nickserv2)
{
    m_plugin->onConnect(m_irccd, ConnectEvent{m_nickserv2});

    ASSERT_EQ("NickServ:identify jean something", m_nickserv2->last());
}

TEST_F(AuthTest, quakenet)
{
    m_plugin->onConnect(m_irccd, ConnectEvent{m_quakenet});

    ASSERT_EQ("Q@CServe.quakenet.org:AUTH mario hello", m_quakenet->last());
}

int main(int argc, char **argv)
{
    path::setApplicationPath(argv[0]);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
