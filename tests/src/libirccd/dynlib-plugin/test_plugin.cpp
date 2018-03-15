/*
 * test_plugin.cpp -- basic exported plugin test
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

#include <boost/dll.hpp>

#include <irccd/daemon/plugin.hpp>

namespace irccd {

class test_plugin : public plugin {
public:
    using plugin::plugin;

    void handle_command(irccd&, const message_event& event) override
    {
        event.server->message("test", "handle_command");
    }

    void handle_connect(irccd&, const connect_event& event) override
    {
        event.server->message("test", "handle_connect");
    }

    void handle_invite(irccd&, const invite_event& event) override
    {
        event.server->message("test", "handle_invite");
    }

    void handle_join(irccd&, const join_event& event) override
    {
        event.server->message("test", "handle_join");
    }

    void handle_kick(irccd&, const kick_event& event) override
    {
        event.server->message("test", "handle_kick");
    }

    void handle_message(irccd&, const message_event& event) override
    {
        event.server->message("test", "handle_message");
    }

    void handle_me(irccd&, const me_event& event) override
    {
        event.server->message("test", "handle_me");
    }

    void handle_mode(irccd&, const mode_event& event) override
    {
        event.server->message("test", "handle_mode");
    }

    void handle_names(irccd&, const names_event& event) override
    {
        event.server->message("test", "handle_names");
    }

    void handle_nick(irccd&, const nick_event& event) override
    {
        event.server->message("test", "handle_nick");
    }

    void handle_notice(irccd&, const notice_event& event) override
    {
        event.server->message("test", "handle_notice");
    }

    void handle_part(irccd&, const part_event& event) override
    {
        event.server->message("test", "handle_part");
    }

    void handle_topic(irccd&, const topic_event& event) override
    {
        event.server->message("test", "handle_topic");
    }

    void handle_whois(irccd&, const whois_event& event) override
    {
        event.server->message("test", "handle_whois");
    }
};

} // !irccd

extern "C" {

BOOST_SYMBOL_EXPORT
std::unique_ptr<irccd::plugin> irccd_testplugin_load(std::string name, std::string path)
{
    return std::make_unique<irccd::test_plugin>(name, path);
}

} // !C
