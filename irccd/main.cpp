/*
 * main.cpp -- irccd main file
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include "sysconfig.hpp"

#include <csignal>
#include <iostream>

#include <boost/asio.hpp>

#include <irccd/config.hpp>
#include <irccd/options.hpp>
#include <irccd/system.hpp>

#include <irccd/daemon/dynlib_plugin.hpp>
#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/transport_command.hpp>
#include <irccd/daemon/transport_service.hpp>

#if defined(IRCCD_HAVE_JS)
#	include <irccd/js/api.hpp>
#	include <irccd/js/plugin.hpp>
#endif

namespace irccd::daemon {

namespace {

std::unique_ptr<bot> instance;

// {{{ usage

[[noreturn]]
void usage()
{
	std::cerr << "usage: irccd [-v] [-c config]\n";
	std::cerr << "       irccd info\n";
	std::cerr << "       irccd version\n";
	std::exit(1);
}

// }}}

// {{{ info

void info()
{
	bool ssl = false;
	bool js = false;

#if defined(IRCCD_HAVE_SSL)
	ssl = true;
#endif
#if defined(IRCCD_HAVE_JS)
	js = true;
#endif

	std::cout << std::boolalpha;
	std::cout << "ssl:              " << ssl << std::endl;
	std::cout << "javascript:       " << js << std::endl;

	std::exit(0);
}

// }}}

// {{{ version

void version()
{
	std::cout << IRCCD_VERSION << std::endl;
	std::exit(0);
}

// }}}

// {{{ init

void init()
{
	// Needed for some components.
	sys::set_program_name("irccd");

	// Default logging to console.
	instance->get_log().set_verbose(false);
}

// }}}

// {{{ parse

auto parse(int argc, char** argv) -> options::pack
{
	options::pack result;

	try {
		result = options::parse(argc, argv, "c:v");

		for (const auto& [ opt, _ ] : std::get<1>(result))
			if (opt == 'v')
				instance->get_log().set_verbose(true);
	} catch (const std::exception& ex) {
		instance->get_log().warning("irccd", "") << "abort: " << ex.what() << std::endl;
		usage();
	}

	return result;
}

// }}}

// {{{ open

auto open(const options::pack& result) -> config
{
	const auto& [ _, options ] = result;

	if (const auto it = options.find('c'); it != options.end())
		return config(it->second);

	const auto cfg = config::search("irccd.conf");

	if (!cfg)
		throw std::runtime_error("no configuration file could be found");

	return *cfg;
}

// }}}

} // !namespace

} // !irccd::daemon

int main(int argc, char** argv)
{
	--argc;
	++argv;

	using namespace irccd;
	using namespace irccd::daemon;

	boost::asio::io_service service;
	boost::asio::signal_set sigs(service, SIGINT, SIGTERM);

	instance = std::make_unique<bot>(service);

	init();

	// 1. Load commands.
	for (const auto& f : transport_command::registry())
		instance->get_transports().get_commands().push_back(f());

	// 2. Load plugin loaders.
	instance->get_plugins().add_loader(std::make_unique<dynlib_plugin_loader>());

#if defined(IRCCD_HAVE_JS)
	auto loader = std::make_unique<js::plugin_loader>(*instance);

	for (const auto& f : js::api::registry())
		loader->get_modules().push_back(f());

	instance->get_plugins().add_loader(std::move(loader));
#endif

	const auto options = parse(argc, argv);

	if (argc > 0) {
		if (std::strcmp(argv[0], "info") == 0)
			irccd::daemon::info();
			// NOTREACHED
		if (std::strcmp(argv[0], "version") == 0)
			irccd::daemon::version();
			// NOTREACHED
	}


	try {
		instance->set_config(open(options));
		instance->load();
	} catch (const std::exception& ex) {
		std::cerr << "abort: " << ex.what() << std::endl;
		return 1;
	}

	/*
	 * Assign instance to nullptr to force deletion of irccd and all its
	 * associated objects before any other static global values such as
	 * loggers.
	 */
	sigs.async_wait([&] (auto, auto) {
		service.stop();
	});

	try {
		service.run();
	} catch (const std::exception& ex) {
		std::cerr << "abort: " << ex.what() << std::endl;
		return 1;
	}

	instance = nullptr;
}
