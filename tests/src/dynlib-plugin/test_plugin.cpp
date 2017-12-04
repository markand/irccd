/*
 * test_plugin.cpp -- basic exported plugin test
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

#include <boost/dll.hpp>

#include <plugin.hpp>

namespace irccd {

class test_plugin : public plugin {
public:
    using plugin::plugin;

    void on_command(irccd&, const message_event& event) override
    {
        event.server->message("test", "on_command");
    }

    void on_connect(irccd&, const connect_event& event) override
    {
        event.server->message("test", "on_connect");
    }

    void on_channel_mode(irccd&, const channel_mode_event& event) override
    {
        event.server->message("test", "on_channel_mode");
    }

    void on_channel_notice(irccd&, const channel_notice_event& event) override
    {
        event.server->message("test", "on_channel_notice");
    }

    void on_invite(irccd&, const invite_event& event) override
    {
        event.server->message("test", "on_invite");
    }

    void on_join(irccd&, const join_event& event) override
    {
        event.server->message("test", "on_join");
    }

    void on_kick(irccd&, const kick_event& event) override
    {
        event.server->message("test", "on_kick");
    }

    void on_message(irccd&, const message_event& event) override
    {
        event.server->message("test", "on_message");
    }

    void on_me(irccd&, const me_event& event) override
    {
        event.server->message("test", "on_me");
    }

    void on_mode(irccd&, const mode_event& event) override
    {
        event.server->message("test", "on_mode");
    }

    void on_names(irccd&, const names_event& event) override
    {
        event.server->message("test", "on_names");
    }

    void on_nick(irccd&, const nick_event& event) override
    {
        event.server->message("test", "on_nick");
    }

    void on_notice(irccd&, const notice_event& event) override
    {
        event.server->message("test", "on_notice");
    }

    void on_part(irccd&, const part_event& event) override
    {
        event.server->message("test", "on_part");
    }

    void on_query(irccd&, const query_event& event) override
    {
        event.server->message("test", "on_query");
    }

    void on_query_command(irccd&, const query_event& event) override
    {
        event.server->message("test", "on_query_command");
    }

    void on_topic(irccd&, const topic_event& event) override
    {
        event.server->message("test", "on_topic");
    }

    void on_whois(irccd&, const whois_event& event) override
    {
        event.server->message("test", "on_whois");
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
