/*
 * irccd.cpp -- main irccd class
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

#include "irccd.hpp"
#include "logger.hpp"
#include "net.hpp"
#include "service.hpp"
#include "util.hpp"

using namespace std;
using namespace std::placeholders;
using namespace std::string_literals;

namespace irccd {

Irccd::Irccd()
    : m_commandService(std::make_shared<CommandService>())
    , m_interruptService(std::make_shared<InterruptService>())
    , m_servers(std::make_shared<ServerService>(*this))
    , m_transports(std::make_shared<TransportService>(*this))
    , m_ruleService(std::make_shared<RuleService>())
    , m_plugins(std::make_shared<PluginService>(*this))
{
}

void Irccd::post(std::function<void (Irccd &)> ev) noexcept
{
    std::lock_guard<mutex> lock(m_mutex);

    m_events.push_back(move(ev));
    m_interruptService->interrupt();
}

void Irccd::run()
{
    while (m_running)
        util::poller::poll(250, *this);
}

void Irccd::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    util::poller::prepare(in, out, max, *m_interruptService, *m_servers, *m_transports);
}

void Irccd::sync(fd_set &in, fd_set &out)
{
    util::poller::sync(in, out, *m_interruptService, *m_servers, *m_transports);

    /*
     * Make a copy because the events can add other events while we are
     * iterating it. Also lock because the timers may alter these events too.
     */
    std::vector<std::function<void (Irccd &)>> copy;

    {
        std::lock_guard<mutex> lock(m_mutex);

        copy = move(m_events);
        m_events.clear();
    }

    if (copy.size() > 0)
        log::debug() << "irccd: dispatching " << copy.size() << " event" << (copy.size() > 1 ? "s" : "") << endl;

    for (auto &ev : copy)
        ev(*this);
}

void Irccd::stop()
{
    log::debug() << "irccd: requesting to stop now" << endl;

    m_running = false;
    m_interruptService->interrupt();
}

} // !irccd
