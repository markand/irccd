/*
 * main.cpp -- irccd controller main
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

#include "command-help.h"
#include "command-watch.h"

#include "command-plugin-info.h"
#include "command-plugin-list.h"
#include "command-plugin-load.h"
#include "command-plugin-reload.h"
#include "command-plugin-unload.h"

#include "command-server-cmode.h"
#include "command-server-cnotice.h"
#include "command-server-connect.h"
#include "command-server-disconnect.h"
#include "command-server-info.h"
#include "command-server-invite.h"
#include "command-server-join.h"
#include "command-server-kick.h"
#include "command-server-list.h"
#include "command-server-me.h"
#include "command-server-message.h"
#include "command-server-mode.h"
#include "command-server-nick.h"
#include "command-server-notice.h"
#include "command-server-part.h"
#include "command-server-reconnect.h"
#include "command-server-topic.h"

#include "irccdctl.h"
#include "path.h"

using namespace irccd;
using namespace irccd::command;

int main(int argc, char **argv)
{
	sys::setProgramName("irccdctl");
	path::setApplicationPath(argv[0]);
	log::setInterface(std::make_unique<log::Console>());
	log::setVerbose(false);
	net::init();

	try {
		Irccdctl ctl;

		ctl.add<Help>("help");
		ctl.add<PluginInfo>("plugin-info");
		ctl.add<PluginList>("plugin-list");
		ctl.add<PluginLoad>("plugin-load");
		ctl.add<PluginReload>("plugin-reload");
		ctl.add<PluginUnload>("plugin-unload");
		ctl.add<ServerChannelMode>("server-cmode");
		ctl.add<ServerChannelNotice>("server-cnotice");
		ctl.add<ServerConnect>("server-connect");
		ctl.add<ServerDisconnect>("server-disconnect");
		ctl.add<ServerInfo>("server-info");
		ctl.add<ServerInvite>("server-invite");
		ctl.add<ServerJoin>("server-join");
		ctl.add<ServerKick>("server-kick");
		ctl.add<ServerList>("server-list");
		ctl.add<ServerMe>("server-me");
		ctl.add<ServerMessage>("server-message");
		ctl.add<ServerMode>("server-mode");
		ctl.add<ServerNick>("server-nick");
		ctl.add<ServerNotice>("server-notice");
		ctl.add<ServerPart>("server-part");
		ctl.add<ServerReconnect>("server-reconnect");
		ctl.add<ServerTopic>("server-topic");
		ctl.add<Watch>("watch");
		ctl.run(--argc, ++argv);
	} catch (const std::exception &ex) {
		log::warning() << sys::programName() << ": " << ex.what() << std::endl;
		std::exit(1);
	}

	return 0;
}
