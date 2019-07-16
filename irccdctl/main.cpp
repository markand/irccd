/*
 * main.cpp -- irccd controller main
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

#include <irccd/sysconfig.hpp>

#include <iostream>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/predef/os.h>

#include <irccd/config.hpp>
#include <irccd/connector.hpp>
#include <irccd/json_util.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include <irccd/daemon/transport_server.hpp>

#include <irccd/ctl/controller.hpp>

#include "alias.hpp"
#include "cli.hpp"

using boost::format;
using boost::str;

using irccd::daemon::transport_error;

namespace irccd::ctl {

namespace {

// Main service;
boost::asio::io_service service;

// Global options.
bool verbose = false;

// Connection to instance.
std::unique_ptr<controller> ctl;

// List of all commands and alias.
std::unordered_map<std::string, alias> aliases;
std::unordered_map<std::string, std::unique_ptr<cli>> commands;

// Vector of commands to execute.
std::vector<std::function<void ()>> requests;

/*
 * Configuration file parsing.
 * -------------------------------------------------------------------
 */

[[noreturn]]
void usage()
{
	std::cerr << "usage: irccdctl plugin-config id [variable] [value]\n";
	std::cerr << "       irccdctl plugin-info id\n";
	std::cerr << "       irccdctl plugin-list\n";
	std::cerr << "       irccdctl plugin-load name\n";
	std::cerr << "       irccdctl plugin-reload plugin plugin-unload plugin\n";
	std::cerr << "       irccdctl rule-add [-c channel] [-e event] [-i index] [-o origin] [-s server] accept|drop\n";
	std::cerr << "       irccdctl rule-edit [-a accept|drop] [-c|C channel] [-e|E event] [-o|O origin] [-s|S server] index\n";
	std::cerr << "       irccdctl rule-info index\n";
	std::cerr << "       irccdctl rule-list\n";
	std::cerr << "       irccdctl rule-move from to\n";
	std::cerr << "       irccdctl rule-remove index\n";
	std::cerr << "       irccdctl server-connect [-46s] [-n nickname] [-r realname] [-u username] [-p port] id hostname\n";
	std::cerr << "       irccdctl server-disconnect [server]\n";
	std::cerr << "       irccdctl server-info server\n";
	std::cerr << "       irccdctl server-invite server target channel\n";
	std::cerr << "       irccdctl server-join server channel [password]\n";
	std::cerr << "       irccdctl server-kick server target channel [reason]\n";
	std::cerr << "       irccdctl server-list\n";
	std::cerr << "       irccdctl server-me server target message\n";
	std::cerr << "       irccdctl server-message server target message\n";
	std::cerr << "       irccdctl server-mode server target mode [limit] [user] [mask]\n";
	std::cerr << "       irccdctl server-nick server nickname\n";
	std::cerr << "       irccdctl server-notice server target message\n";
	std::cerr << "       irccdctl server-part server channel [reason]\n";
	std::cerr << "       irccdctl server-reconnect [server]\n";
	std::cerr << "       irccdctl server-topic server channel topic\n";
	std::cerr << "       irccdctl watch [-f native|json]\n";
	std::exit(1);
}

/*
 * read_connect_ip
 * -------------------------------------------------------------------
 *
 * Extract IP connection information from the config file.
 *
 * [connect]
 * type = "ip"
 * hostname = "ip or hostname"
 * port = "port number or service"
 * ipv4 = try IPv4 (Optional, default: true)
 * ipv6 = try IPv6 (Optional, default: true)
 * ssl = true | false (Optional, default: false)
 */
auto read_connect_ip(const ini::section& sc) -> std::unique_ptr<connector>
{
	const auto hostname = sc.get("hostname").get_value();
	const auto port = sc.get("port").get_value();
	bool ipv4 = true;
	bool ipv6 = true;

	if (const auto it = sc.find("ipv4"); it != sc.end())
		ipv4 = string_util::is_boolean(it->get_value());
	if (const auto it = sc.find("ipv6"); it != sc.end())
		ipv6 = string_util::is_boolean(it->get_value());

	if (!ipv4 && !ipv6)
		throw transport_error(transport_error::invalid_family);
	if (hostname.empty())
		throw transport_error(transport_error::invalid_hostname);
	if (port.empty())
		throw transport_error(transport_error::invalid_port);

	if (string_util::is_boolean(sc.get("ssl").get_value())) {
#if defined(IRCCD_HAVE_SSL)
		// TODO: support more parameters.
		boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);

		return std::make_unique<tls_ip_connector>(std::move(ctx),
			service, hostname, port, ipv4, ipv6);
#else
		throw std::runtime_error("SSL disabled");
#endif
	}

	return std::make_unique<ip_connector>(service, hostname, port, ipv4, ipv6);
}

/*
 * read_connect_local
 * -------------------------------------------------------------------
 *
 * Extract local connection for Unix.
 *
 * [connect]
 * type = "unix"
 * path = "path to socket file"
 */
auto read_connect_local(const ini::section& sc) -> std::unique_ptr<connector>
{
#if !BOOST_OS_WINDOWS
	using boost::asio::local::stream_protocol;

	const auto it = sc.find("path");

	if (it == sc.end())
		throw std::invalid_argument("missing path parameter");

	if (string_util::is_boolean(sc.get("ssl").get_value())) {
#if defined(IRCCD_HAVE_SSL)
		// TODO: support more parameters.
		boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);

		return std::make_unique<tls_local_connector>(std::move(ctx), service, it->get_value());
#else
		throw std::runtime_error("SSL disabled");
#endif
	}

	return std::make_unique<local_connector>(service, it->get_value());
#else
	(void)sc;

	throw std::invalid_argument("unix connection not supported on Windows");
#endif
}

/*
 * read_connect
 * -------------------------------------------------------------------
 *
 * Generic function for reading the [connect] section.
 */
void read_connect(const ini::section& sc)
{
	const auto it = sc.find("type");

	if (it == sc.end())
		throw std::invalid_argument("missing type parameter");

	std::unique_ptr<connector> connector;

	if (it->get_value() == "ip")
		connector = read_connect_ip(sc);
	else if (it->get_value() == "unix")
		connector = read_connect_local(sc);
	else
		throw std::invalid_argument(str(format("invalid type given: %1%") % it->get_value()));

	if (connector) {
		ctl = std::make_unique<controller>(std::move(connector));

		auto password = sc.find("password");

		if (password != sc.end())
			ctl->set_password(password->get_value());
	}
}

/*
 * read_general
 * -------------------------------------------------------------------
 *
 * Read the general section.
 *
 * [general]
 * verbose = true
 */
void read_general(const ini::section& sc)
{
	const auto value = sc.find("verbose");

	if (value != sc.end())
		verbose = string_util::is_boolean(value->get_value());
}

/*
 * read_alias
 * -------------------------------------------------------------------
 *
 * Read aliases for irccdctl.
 *
 * [alias.<name>]
 * cmd1 = ( "command", "arg1, "...", "argn" )
 * cmd2 = ( "command", "arg1, "...", "argn" )
 */
auto read_alias(const ini::section& sc, const std::string& name) -> alias
{
	alias alias(name);

	/*
	 * Each defined option is a command that the user can call. The name is
	 * unused and serves as documentation purpose.
	 */
	for (const auto& option : sc) {
		/*
		 * Iterate over the arguments which are usually a list and the first
		 * argument is a command name.
		 */
		if (option.size() == 1 && option[0].empty())
			throw std::runtime_error(str(format("alias %1%: missing command name in '%2%'")
				% name % option.get_key()));

		std::string command = option[0];
		std::vector<alias_arg> args(option.begin() + 1, option.end());

		alias.emplace_back(std::move(command), std::move(args));
	}

	return alias;
}

void read(const config& cfg)
{
	ini::document::const_iterator it;

	if (!ctl && (it = cfg.find("connect")) != cfg.end())
		read_connect(*it);
	if ((it = cfg.find("general")) != cfg.end())
		read_general(*it);

	// [alias.*] sections.
	for (const auto& sc : cfg) {
		if (sc.get_key().compare(0, 6, "alias.") == 0) {
			auto name = sc.get_key().substr(6);
			auto alias = read_alias(sc, name);

			aliases.emplace(std::move(name), std::move(alias));
		}
	}
}

/*
 * Command line parsing.
 * -------------------------------------------------------------------
 */

/*
 * parse_connect_ip
 * ------------------------------------------------------------------
 *
 * Parse internet connection from command line.
 *
 * -h hostname or ip address
 * -p port (can be a string)
 * -4 enable IPv4 (default)
 * -6 enable IPv6 (default)
 */
auto parse_connect_ip(const options::pack& result) -> std::unique_ptr<connector>
{
	const auto& [ _, options ] = result;
	const auto hostname = options.find('h');
	const auto port = options.find('p');

	/*
	 * Both are to true by default, setting one disable the second unless
	 * it is also specified.
	 */
	bool ipv4 = true;
	bool ipv6 = true;

	if (options.count('4'))
		ipv6 = options.count('6');
	else if (options.count('6'))
		ipv4 = options.count('4');

	if (hostname == options.end() || hostname->second.empty())
		throw transport_error(transport_error::invalid_hostname);
	if (port == options.end() || port->second.empty())
		throw transport_error(transport_error::invalid_port);

	return std::make_unique<ip_connector>(service, hostname->second, port->second, ipv4, ipv6);
}

/*
 * parse_connect_local
 * ------------------------------------------------------------------
 *
 * Parse local connection.
 *
 * -P file
 */
auto parse_connect_local([[maybe_unused]] const options::pack& options) -> std::unique_ptr<connector>
{
#if !BOOST_OS_WINDOWS
	const auto& [ _, options ] = result;
	const auto path = options.find('P');

	if (path == options.end() || path->second.empty())
		throw transport_error(transport_error::invalid_path);

	return std::make_unique<local_connector>(service, it->second);
#else
	throw transport_error(transport_error::not_supported);
#endif
}

/*
 * parse_connect
 * ------------------------------------------------------------------
 *
 * Generic parsing of command line option for connection.
 */
void parse_connect(const options::pack& options)
{
	const auto hflag = std::get<1>(options).count('h') > 0;
	const auto pflag = std::get<1>(options).count('P') > 0;

	if (hflag && pflag)
		throw std::invalid_argument("-h and -P are mutually exclusive");

	if (hflag)
		ctl = std::make_unique<controller>(parse_connect_ip(options));
	else if (pflag)
		ctl = std::make_unique<controller>(parse_connect_local(options));
}

auto parse(std::vector<std::string>& args) -> options::pack
{
	options::pack result;

	try {
		// 1. Collect the options before the command name.
		auto begin = args.begin();
		auto end = args.end();

		result = options::parse(begin, end, "c:h:p:P:v!");
	
		for (const auto& [ opt, _ ] : std::get<1>(result))
			if (opt == 'v')
				verbose = true;

		args.erase(args.begin(), begin);
	} catch (const std::exception& ex) {
		std::cerr << "abort: " << ex.what() << std::endl;
		usage();
	}

	return result;
}

void enqueue(std::vector<std::string>);

void enqueue(const alias& alias, std::vector<std::string> args_copy)
{
	for (const auto& cmd : alias) {
		std::vector<std::string> args(args_copy);
		std::vector<std::string> cmd_args;
		std::vector<std::string>::size_type toremove = 0;

		// 1. Append command name before.
		cmd_args.push_back(cmd.get_command());

		for (const auto& arg : cmd.get_args()) {
			if (arg.is_placeholder()) {
				if (args.size() < arg.get_index() + 1)
					throw std::invalid_argument(
						str(format("missing argument for placeholder %1%") % arg.get_index()));

				cmd_args.push_back(args[arg.get_index()]);

				if (arg.get_index() + 1 > toremove)
					toremove = arg.get_index() + 1;
			} else
				cmd_args.push_back(arg.get_value());
		}

		assert(toremove <= args.size());

		// 2. Remove the arguments that been placed in placeholders.
		args.erase(args.begin(), args.begin() + toremove);

		// 3. Now append the rest of arguments.
		std::copy(args.begin(), args.end(), std::back_inserter(cmd_args));

		// 4. Finally try to execute.
		enqueue(cmd_args);
	}
}

void enqueue(std::vector<std::string> args)
{
	assert(args.size() > 0);

	auto name = args[0];
	auto alias = aliases.find(name);

	// Remove name.
	args.erase(args.begin());

	if (alias != aliases.end()) {
		enqueue(alias->second, args);
		return;
	}

	const auto cmd = commands.find(name);

	if (cmd != commands.end())
		requests.push_back([args, cmd] () {
			cmd->second->exec(*ctl, args);
		});
	else
		throw std::invalid_argument("no alias or command named " + name);
}

void init()
{
	sys::set_program_name("irccdctl");

	for (const auto& f : cli::registry) {
		auto c = f();

		commands.emplace(c->get_name(), std::move(c));
	}
}

void do_connect()
{
	ctl->connect([&] (auto code, auto info) {
		if (code)
			throw std::system_error(code);

		if (verbose) {
			const json_util::deserializer doc(info);
			const auto major = doc.get<int>("major");
			const auto minor = doc.get<int>("minor");
			const auto patch = doc.get<int>("patch");

			if (!major || !minor || !patch)
				std::cout << "connected to irccd (unknown version)" << std::endl;
			else
				std::cout << "connected to irccd "
				          << *major << "."
				          << *minor << "."
				          << *patch << std::endl;
		}
	});

	service.run();
	service.reset();
}

void do_exec(const std::vector<std::string>& args)
{
	enqueue(args);

	for (const auto& req : requests) {
		req();
		service.run();
		service.reset();
	}
}

} // !namespace

} // !irccd::ctl

int main(int argc, char** argv)
{
	--argc;
	++argv;

	// 0. Keep track of parsed arguments.
	std::vector<std::string> cli(argc);

	for (int i = 0; i < argc; ++i)
		cli[i] = argv[i];

	irccd::ctl::init();

	// 1. Read command line arguments.
	const auto result = irccd::ctl::parse(cli);
	const auto& [ args, options ] = result;

	/*
	 * 2. Open optional config by command line or by searching it
	 *
	 * The connection to irccd is searched in the following order :
	 *
	 * 1. From the command line if specified
	 * 2. From the configuration file specified by -c
	 * 3. From the configuration file searched through directories
	 */
	try {
		irccd::ctl::parse_connect(result);

		if (const auto it = options.find('c'); it != options.end())
			irccd::ctl::read(it->second);
		else if (const auto conf = irccd::config::search("irccdctl.conf"))
			irccd::ctl::read(*conf);
	} catch (const std::exception& ex) {
		std::cerr << "abort: " << ex.what() << std::endl;
		return 1;
	}

	if (cli.size() <= 0)
		irccd::ctl::usage();
		// NOTREACHED

	if (!irccd::ctl::ctl) {
		std::cerr << "abort: no connection specified" << std::endl;
		return 1;
	}

	try {
		irccd::ctl::do_connect();
		irccd::ctl::do_exec(cli);
	} catch (const std::system_error& ex) {
		std::cerr << "abort: " << ex.code().message() << std::endl;
		return 1;
	} catch (const std::exception& ex) {
		std::cerr << "abort: " << ex.what() << std::endl;
		return 1;
	}
}
