/*
 * mock_server.cpp -- mock server
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

#include "mock_server.hpp"

namespace irccd {

void mock_server::connect(connect_handler) noexcept
{
    push("connect");
}

void mock_server::disconnect() noexcept
{
    push("disconnect");
}

void mock_server::invite(std::string_view target, std::string_view channel)
{
    push("invite", { std::string(target), std::string(channel) });
}

void mock_server::join(std::string_view channel, std::string_view password)
{
    push("join", { std::string(channel), std::string(password) });
}

void mock_server::kick(std::string_view target, std::string_view channel, std::string_view reason)
{
    push("kick", { std::string(target), std::string(channel), std::string(reason) });
}

void mock_server::me(std::string_view target, std::string_view message)
{
    push("me", { std::string(target), std::string(message) });
}

void mock_server::message(std::string_view target, std::string_view message)
{
    push("message", { std::string(target), std::string(message) });
}

void mock_server::mode(std::string_view channel,
                       std::string_view mode,
                       std::string_view limit,
                       std::string_view user,
                       std::string_view mask)
{
    push("mode", {
        std::string(channel),
        std::string(mode),
        std::string(limit),
        std::string(user),
        std::string(mask)
    });
}

void mock_server::names(std::string_view channel)
{
    push("names", { std::string(channel) });
}

void mock_server::notice(std::string_view target, std::string_view message)
{
    push("notice", { std::string(target), std::string(message) });
}

void mock_server::part(std::string_view channel, std::string_view reason)
{
    push("part", { std::string(channel), std::string(reason) });
}

void mock_server::send(std::string_view raw)
{
    push("send", { std::string(raw) });
}

void mock_server::topic(std::string_view channel, std::string_view topic)
{
    push("topic", { std::string(channel), std::string(topic) });
}

void mock_server::whois(std::string_view target)
{
    push("whois", { std::string(target) });
}

} // !irccd
