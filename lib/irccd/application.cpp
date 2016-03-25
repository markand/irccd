/*
 * application.cpp -- super base class to create irccd front ends
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

#include "application.h"

#include "command/help.h"
#include "command/plugin-info.h"
#include "command/plugin-list.h"
#include "command/plugin-load.h"
#include "command/plugin-reload.h"
#include "command/plugin-unload.h"
#include "command/server-cmode.h"
#include "command/server-cnotice.h"
#include "command/server-connect.h"
#include "command/server-disconnect.h"
#include "command/server-info.h"
#include "command/server-invite.h"
#include "command/server-join.h"
#include "command/server-kick.h"
#include "command/server-list.h"
#include "command/server-me.h"
#include "command/server-message.h"
#include "command/server-mode.h"
#include "command/server-nick.h"
#include "command/server-notice.h"
#include "command/server-part.h"
#include "command/server-reconnect.h"
#include "command/server-topic.h"
#include "command/watch.h"

namespace irccd {

Application::Application()
{
	/* Register all commands */
	addCommand(std::make_unique<command::Help>());
	addCommand(std::make_unique<command::PluginInfo>());
	addCommand(std::make_unique<command::PluginList>());
	addCommand(std::make_unique<command::PluginLoad>());
	addCommand(std::make_unique<command::PluginReload>());
	addCommand(std::make_unique<command::PluginUnload>());
	addCommand(std::make_unique<command::ServerChannelMode>());
	addCommand(std::make_unique<command::ServerChannelNotice>());
	addCommand(std::make_unique<command::ServerConnect>());
	addCommand(std::make_unique<command::ServerDisconnect>());
	addCommand(std::make_unique<command::ServerInfo>());
	addCommand(std::make_unique<command::ServerInvite>());
	addCommand(std::make_unique<command::ServerJoin>());
	addCommand(std::make_unique<command::ServerKick>());
	addCommand(std::make_unique<command::ServerList>());
	addCommand(std::make_unique<command::ServerMe>());
	addCommand(std::make_unique<command::ServerMessage>());
	addCommand(std::make_unique<command::ServerMode>());
	addCommand(std::make_unique<command::ServerNick>());
	addCommand(std::make_unique<command::ServerNotice>());
	addCommand(std::make_unique<command::ServerPart>());
	addCommand(std::make_unique<command::ServerReconnect>());
	addCommand(std::make_unique<command::ServerTopic>());
	addCommand(std::make_unique<command::Watch>());
}

} // !irccd
