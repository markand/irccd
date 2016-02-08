/*
 * main.cpp -- irccd main file
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

#include <csignal>

#include <irccd-config.h>

#include <logger.h>
#include <options.h>
#include <path.h>
#include <system.h>

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
#include "config.h"
#include "irccd.h"

using namespace irccd;

namespace {

std::unique_ptr<Irccd> instance;

void stop(int)
{
	instance->stop();
}

void init(int &argc, char **&argv)
{
	/* Needed for some components */
	sys::setProgramName("irccd");
	path::setApplicationPath(argv[0]);

	/* Default logging to console */
	log::setVerbose(false);
	log::setInterface(std::make_unique<log::Console>());

	signal(SIGINT, stop);
	signal(SIGTERM, stop);

	-- argc;
	++ argv;
}

void usage()
{
	log::warning() << "usage: " << sys::programName() << " [options...]\n\n";
	log::warning() << "Available options:\n";
	log::warning() << "  -c, --config file       specify the configuration file\n";
	log::warning() << "  -f, --foreground        do not run as a daemon\n";
	log::warning() << "  --help                  show this help\n";
	log::warning() << "  -p, --plugin name       load a specific plugin\n";
	log::warning() << "  -v, --verbose           be verbose" << std::endl;
	std::exit(1);
}

} // !namespace

int main(int argc, char **argv)
{
	init(argc, argv);

	/* Parse command line options */
	parser::Result result;

	try {
		parser::Options options{
			{ "-c",			true	},
			{ "--config",		true	},
			{ "-f",			false	},
			{ "--foreground",	false	},
			{ "--help",		false	},
			{ "-p",			true	},
			{ "--plugin",		true	},
			{ "-v",			false	},
			{ "--verbose",		false	}
		};

		result = parser::read(argc, argv, options);

		for (const auto &pair : result) {
			if (pair.first == "--help")
				usage();
				// NOTREACHED
	
			if (pair.first == "-v" || pair.first == "--verbose")
				log::setVerbose(true);
		}
	} catch (const std::exception &ex) {
		log::warning() << sys::programName() << ": " << ex.what() << std::endl;
		usage();
	}

	instance = std::make_unique<Irccd>();
	instance->addTransportCommand<command::PluginInfo>("plugin-info");
	instance->addTransportCommand<command::PluginList>("plugin-list");
	instance->addTransportCommand<command::PluginLoad>("plugin-load");
	instance->addTransportCommand<command::PluginReload>("plugin-reload");
	instance->addTransportCommand<command::PluginUnload>("plugin-unload");
	instance->addTransportCommand<command::ServerChannelMode>("server-cmode");
	instance->addTransportCommand<command::ServerChannelNotice>("server-cnotice");
	instance->addTransportCommand<command::ServerConnect>("server-connect");
	instance->addTransportCommand<command::ServerDisconnect>("server-disconnect");
	instance->addTransportCommand<command::ServerInfo>("server-info");
	instance->addTransportCommand<command::ServerInvite>("server-invite");
	instance->addTransportCommand<command::ServerJoin>("server-join");
	instance->addTransportCommand<command::ServerKick>("server-kick");
	instance->addTransportCommand<command::ServerList>("server-list");
	instance->addTransportCommand<command::ServerMe>("server-me");
	instance->addTransportCommand<command::ServerMessage>("server-message");
	instance->addTransportCommand<command::ServerMode>("server-mode");
	instance->addTransportCommand<command::ServerNick>("server-nick");
	instance->addTransportCommand<command::ServerNotice>("server-notice");
	instance->addTransportCommand<command::ServerPart>("server-part");
	instance->addTransportCommand<command::ServerReconnect>("server-reconnect");
	instance->addTransportCommand<command::ServerTopic>("server-topic");
	instance->load(Config{result});
	instance->run();

	return 0;
}
