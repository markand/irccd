/*
 * command-tester.cpp -- test fixture helper for remote commands
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

#include "command-tester.hpp"
#include "client.hpp"
#include "logger.hpp"
#include "service.hpp"
#include "transport.hpp"

namespace irccd {

CommandTester::CommandTester(std::unique_ptr<command> cmd,
                             std::unique_ptr<server> server)
    : m_irccdctl(std::make_unique<Client>())
{
    // Be silent.
    log::set_logger(std::make_unique<log::silent_logger>());
    log::set_verbose(false);

    auto tpt = std::make_unique<transport_server_ip>("*", 0);
    auto port = tpt->port();

    m_irccd.transports().add(std::move(tpt));
    m_irccdctl.client().connect(net::ipv4::pton("127.0.0.1", port));

    if (cmd)
        m_irccd.commands().add(std::move(cmd));
    if (server)
        m_irccd.servers().add(std::move(server));
}

} // !irccd
