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

void debug_server::connect() noexcept
{
    std::cout << get_name() << ": connect" << std::endl;
}

void debug_server::disconnect() noexcept
{
    std::cout << get_name() << ": disconnect" << std::endl;
}

void debug_server::reconnect() noexcept
{
    std::cout << get_name() << ": reconnect" << std::endl;
}

void debug_server::invite(std::string target, std::string channel)
{
    std::cout << get_name() << ": invite " << target << " " << channel << std::endl;
}

void debug_server::join(std::string channel, std::string password)
{
    std::cout << get_name() << ": join " << channel << " " << password << std::endl;
}

void debug_server::kick(std::string target, std::string channel, std::string reason)
{
    std::cout << get_name() << ": kick " << target << " " << channel << " " << reason << std::endl;
}

void debug_server::me(std::string target, std::string message)
{
    std::cout << get_name() << ": me " << target << " " << message << std::endl;
}

void debug_server::message(std::string target, std::string message)
{
    std::cout << get_name() << ": message " << target << " " << message << std::endl;
}

void debug_server::mode(std::string channel,
          std::string mode,
          std::string limit,
          std::string user,
          std::string mask)
{
    std::cout << get_name() << ": mode "
              << channel << " "
              << mode << " "
              << limit << " "
              << user << " "
              << mask << std::endl;
}

void debug_server::names(std::string channel)
{
    std::cout << get_name() << ": names " << channel << std::endl;
}

void debug_server::notice(std::string target, std::string message)
{
    std::cout << get_name() << ": notice " << target << " " << message << std::endl;
}

void debug_server::part(std::string channel, std::string reason)
{
    std::cout << get_name() << ": part " << channel << " " << reason << std::endl;
}

void debug_server::send(std::string raw)
{
    std::cout << get_name() << ": send " << raw << std::endl;
}

void debug_server::topic(std::string channel, std::string topic)
{
    std::cout << get_name() << ": topic " << channel << " " << topic << std::endl;
}

void debug_server::whois(std::string target)
{
    std::cout << get_name() << ": whois " << target << std::endl;
}

} // !irccd
