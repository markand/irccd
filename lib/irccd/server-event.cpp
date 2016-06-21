/*
 * server-event.cpp -- server event
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

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin-js.hpp"
#include "server-event.hpp"
#include "service-plugin.hpp"
#include "service-rule.hpp"

namespace irccd {

ServerEvent::ServerEvent(std::string server,
                         std::string origin,
                         std::string target,
                         std::function<std::string (Plugin &)> functionName,
                         std::function<void (Plugin &)> exec)
    : m_server(std::move(server))
    , m_origin(std::move(origin))
    , m_target(std::move(target))
    , m_functionName(std::move(functionName))
    , m_exec(std::move(exec))
{
}

void ServerEvent::operator()(Irccd &irccd) const
{
    for (auto &plugin : irccd.pluginService().plugins()) {
        auto eventname = m_functionName(*plugin);
        auto allowed = irccd.ruleService().solve(m_server, m_target, m_origin, plugin->name(), eventname);

        if (!allowed) {
            log::debug() << "rule: event skipped on match" << std::endl;
            continue;
        } else
            log::debug() << "rule: event allowed" << std::endl;

        // TODO: server-event must not know which type of plugin.
        // TODO: get generic error.
        try {
            m_exec(*plugin);
        } catch (const Exception &info) {
            log::warning() << "plugin " << plugin->name() << ": error: " << info.what() << std::endl;

            if (!info.fileName.empty())
                log::warning() << "    " << info.fileName << ":" << info.lineNumber << std::endl;
            if (!info.stack.empty())
                log::warning() << "    " << info.stack << std::endl;
        }
    }
}

} // !irccd
