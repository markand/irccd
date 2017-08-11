/*
 * main.cpp -- test auth plugin
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

#include <irccd/irccd.hpp>
#include <irccd/server.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

using namespace irccd;

class server_test : public Server {
private:
    std::string last_;

public:
    inline server_test(std::string name)
        : Server(std::move(name))
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

class auth_test : public plugin_test {
protected:
    std::shared_ptr<server_test> nickserv1_;
    std::shared_ptr<server_test> nickserv2_;
    std::shared_ptr<server_test> quakenet_;

public:
    auth_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
        , nickserv1_(std::make_shared<server_test>("nickserv1"))
        , nickserv2_(std::make_shared<server_test>("nickserv2"))
        , quakenet_(std::make_shared<server_test>("quakenet"))
    {
        plugin_->set_config({
            { "nickserv1.type", "nickserv" },
            { "nickserv1.password", "plopation" },
            { "nickserv2.type", "nickserv" },
            { "nickserv2.password", "something" },
            { "nickserv2.username", "jean" },
            { "quakenet.type", "quakenet" },
            { "quakenet.password", "hello" },
            { "quakenet.username", "mario" }
        });
        plugin_->on_load(irccd_);
    }
};

TEST_F(auth_test, nickserv1)
{
    plugin_->on_connect(irccd_, ConnectEvent{nickserv1_});

    ASSERT_EQ("NickServ:identify plopation", nickserv1_->last());
}

TEST_F(auth_test, nickserv2)
{
    plugin_->on_connect(irccd_, ConnectEvent{nickserv2_});

    ASSERT_EQ("NickServ:identify jean something", nickserv2_->last());
}

TEST_F(auth_test, quakenet)
{
    plugin_->on_connect(irccd_, ConnectEvent{quakenet_});

    ASSERT_EQ("Q@CServe.quakenet.org:AUTH mario hello", quakenet_->last());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
