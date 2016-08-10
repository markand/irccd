/*
 * conn-state-checking.cpp -- verify irccd instance
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

#include <format.h>

#include "conn-state-checking.hpp"
#include "conn-state-disconnected.hpp"
#include "conn-state-ready.hpp"
#include "sysconfig.hpp"

using namespace fmt::literals;

namespace irccd {

void Connection::CheckingState::verifyProgram(const nlohmann::json &json) const
{
    auto prog = json.find("program");

    if (prog == json.end() || !prog->is_string() || prog->get<std::string>() != "irccd")
        throw std::runtime_error("not an irccd instance");
}

void Connection::CheckingState::verifyVersion(Connection &cnx, const nlohmann::json &json) const
{
    auto getVersionVar = [&] (auto key) {
        auto it = json.find(key);

        if (it == json.end() || !it->is_number_unsigned())
            throw std::runtime_error("invalid irccd instance");

        return *it;
    };

    Info info{
        getVersionVar("major"),
        getVersionVar("minor"),
        getVersionVar("patch")
    };

    // Ensure compatibility.
    if (info.major != IRCCD_VERSION_MAJOR || info.minor > IRCCD_VERSION_MINOR)
        throw std::runtime_error("server version too recent {}.{}.{} vs {}.{}.{}"_format(
            info.major, info.minor, info.patch,
            IRCCD_VERSION_MAJOR, IRCCD_VERSION_MINOR, IRCCD_VERSION_PATCH));

    // Successfully connected.
    cnx.m_stateNext = std::make_unique<ReadyState>();
    cnx.onConnect(info);
}

void Connection::CheckingState::verify(Connection &cnx) const
{
    auto msg = util::nextNetwork(cnx.m_input);

    if (msg.empty())
        return;

    try {
        auto json = nlohmann::json::parse(msg);

        verifyProgram(json);
        verifyVersion(cnx, json);
    } catch (const std::exception &ex) {
        cnx.m_stateNext = std::make_unique<DisconnectedState>();
        cnx.onDisconnect(ex.what());
    }
}

Connection::Status Connection::CheckingState::status() const noexcept
{
    return Checking;
}

void Connection::CheckingState::prepare(Connection &cnx, fd_set &in, fd_set &)
{
    FD_SET(cnx.m_socket.handle(), &in);
}

void Connection::CheckingState::sync(Connection &cnx, fd_set &, fd_set &)
{
    cnx.syncInput();

    verify(cnx);
}

} // !irccd
