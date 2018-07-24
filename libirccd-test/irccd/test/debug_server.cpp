/*
 * debug_server.cpp -- server which prints everything in the console
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

#include <iostream>

#include "debug_server.hpp"

namespace irccd {

void debug_server::connect(connect_handler) noexcept
{
    std::cout << get_id() << ": connect" << std::endl;
}

void debug_server::disconnect() noexcept
{
    std::cout << get_id() << ": disconnect" << std::endl;
}

void debug_server::invite(std::string_view target, std::string_view channel)
{
    std::cout << get_id() << ": invite " << target << " " << channel << std::endl;
}

void debug_server::join(std::string_view channel, std::string_view password)
{
    std::cout << get_id() << ": join " << channel << " " << password << std::endl;
}

void debug_server::kick(std::string_view target, std::string_view channel, std::string_view reason)
{
    std::cout << get_id() << ": kick " << target << " " << channel << " " << reason << std::endl;
}

void debug_server::me(std::string_view target, std::string_view message)
{
    std::cout << get_id() << ": me " << target << " " << message << std::endl;
}

void debug_server::message(std::string_view target, std::string_view message)
{
    std::cout << get_id() << ": message " << target << " " << message << std::endl;
}

void debug_server::mode(std::string_view channel,
                        std::string_view mode,
                        std::string_view limit,
                        std::string_view user,
                        std::string_view mask)
{
    std::cout << get_id() << ": mode "
              << channel << " "
              << mode << " "
              << limit << " "
              << user << " "
              << mask << std::endl;
}

void debug_server::names(std::string_view channel)
{
    std::cout << get_id() << ": names " << channel << std::endl;
}

void debug_server::notice(std::string_view target, std::string_view message)
{
    std::cout << get_id() << ": notice " << target << " " << message << std::endl;
}

void debug_server::part(std::string_view channel, std::string_view reason)
{
    std::cout << get_id() << ": part " << channel << " " << reason << std::endl;
}

void debug_server::send(std::string_view raw)
{
    std::cout << get_id() << ": send " << raw << std::endl;
}

void debug_server::topic(std::string_view channel, std::string_view topic)
{
    std::cout << get_id() << ": topic " << channel << " " << topic << std::endl;
}

void debug_server::whois(std::string_view target)
{
    std::cout << get_id() << ": whois " << target << std::endl;
}

} // !irccd
