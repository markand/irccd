/*
 * main.cpp -- test ask plugin
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

class server_test : public server {
private:
    std::string last_;

public:
    inline server_test()
        : server("test")
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

class ask_test : public plugin_test {
protected:
    std::shared_ptr<server_test> server_{new server_test};

public:
    inline ask_test()
        : plugin_test(PLUGIN_NAME, PLUGIN_PATH)
    {
        plugin_->set_config({
            { "file", CMAKE_CURRENT_SOURCE_DIR "/answers.conf" }
        });
        plugin_->on_load(irccd_);
    }
};

TEST_F(ask_test, basic)
{
    bool no = false;
    bool yes = false;

    /*
     * Invoke the plugin 1000 times, it will be very unlucky to not have both
     * answers in that amount of tries.
     */
    for (int i = 0; i < 1000; ++i) {
        plugin_->on_command(irccd_, {server_, "tester", "#dummy", ""});

        if (server_->last() == "#dummy:tester, YES")
            yes = true;
        if (server_->last() == "#dummy:tester, NO")
            no = true;
    }

    ASSERT_TRUE(no);
    ASSERT_TRUE(yes);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
