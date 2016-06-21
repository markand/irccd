/*
 * service-command.cpp -- store remote commands
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

#include <algorithm>

#include "cmd-help.hpp"
#include "cmd-plugin-config.hpp"
#include "cmd-plugin-info.hpp"
#include "cmd-plugin-list.hpp"
#include "cmd-plugin-load.hpp"
#include "cmd-plugin-reload.hpp"
#include "cmd-plugin-unload.hpp"
#include "cmd-server-cmode.hpp"
#include "cmd-server-cnotice.hpp"
#include "cmd-server-connect.hpp"
#include "cmd-server-disconnect.hpp"
#include "cmd-server-info.hpp"
#include "cmd-server-invite.hpp"
#include "cmd-server-join.hpp"
#include "cmd-server-kick.hpp"
#include "cmd-server-list.hpp"
#include "cmd-server-me.hpp"
#include "cmd-server-message.hpp"
#include "cmd-server-mode.hpp"
#include "cmd-server-nick.hpp"
#include "cmd-server-notice.hpp"
#include "cmd-server-part.hpp"
#include "cmd-server-reconnect.hpp"
#include "cmd-server-topic.hpp"
#include "cmd-watch.hpp"
#include "service-command.hpp"

namespace irccd {

CommandService::CommandService()
    : m_commands{
        std::make_shared<command::Help>(),
        std::make_shared<command::PluginConfig>(),
        std::make_shared<command::PluginInfo>(),
        std::make_shared<command::PluginList>(),
        std::make_shared<command::PluginLoad>(),
        std::make_shared<command::PluginReload>(),
        std::make_shared<command::PluginUnload>(),
        std::make_shared<command::ServerChannelMode>(),
        std::make_shared<command::ServerChannelNotice>(),
        std::make_shared<command::ServerConnect>(),
        std::make_shared<command::ServerDisconnect>(),
        std::make_shared<command::ServerInfo>(),
        std::make_shared<command::ServerInvite>(),
        std::make_shared<command::ServerJoin>(),
        std::make_shared<command::ServerKick>(),
        std::make_shared<command::ServerList>(),
        std::make_shared<command::ServerMe>(),
        std::make_shared<command::ServerMessage>(),
        std::make_shared<command::ServerMode>(),
        std::make_shared<command::ServerNick>(),
        std::make_shared<command::ServerNotice>(),
        std::make_shared<command::ServerPart>(),
        std::make_shared<command::ServerReconnect>(),
        std::make_shared<command::ServerTopic>(),
        std::make_shared<command::Watch>(),
    }
{
}

bool CommandService::contains(const std::string &name) const noexcept
{
    return find(name) != nullptr;
}

std::shared_ptr<Command> CommandService::find(const std::string &name) const noexcept
{
    auto it = std::find_if(m_commands.begin(), m_commands.end(), [&] (const auto &cmd) {
        return cmd->name() == name;
    });

    return it == m_commands.end() ? nullptr : *it;
}

} // !irccd
