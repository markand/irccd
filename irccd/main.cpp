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

#include <irccd/logger.h>
#include <irccd/options.h>
#include <irccd/path.h>
#include <irccd/system.h>

#include <irccd/config.h>
#include <irccd/irccd.h>

using namespace irccd;

namespace {

std::unique_ptr<Irccd> instance;

void stop(int)
{
	instance->stop();
}

void init(int &argc, char **&argv)
{
	// MOVE THIS IN Application

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

	instance->load(Config{result});
	instance->run();

	return 0;
}
