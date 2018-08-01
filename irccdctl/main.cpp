/*
 * main.cpp -- irccd controller main
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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
#include <irccd/json_util.hpp>
#include <irccd/options.hpp>
#include <irccd/socket_connector.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#if defined(IRCCD_HAVE_SSL)
#   include <irccd/tls_connector.hpp>
#endif

#include <irccd/daemon/transport_server.hpp>

#include <irccd/ctl/controller.hpp>

#include "plugin_config_cli.hpp"
#include "plugin_info_cli.hpp"
#include "plugin_list_cli.hpp"
#include "plugin_load_cli.hpp"
#include "plugin_reload_cli.hpp"
#include "plugin_unload_cli.hpp"
#include "rule_add_cli.hpp"
#include "rule_edit_cli.hpp"
#include "rule_info_cli.hpp"
#include "rule_list_cli.hpp"
#include "rule_move_cli.hpp"
#include "rule_remove_cli.hpp"
#include "server_connect_cli.hpp"
#include "server_disconnect_cli.hpp"
#include "server_info_cli.hpp"
#include "server_invite_cli.hpp"
#include "server_join_cli.hpp"
#include "server_kick_cli.hpp"
#include "server_list_cli.hpp"
#include "server_me_cli.hpp"
#include "server_message_cli.hpp"
#include "server_mode_cli.hpp"
#include "server_nick_cli.hpp"
#include "server_notice_cli.hpp"
#include "server_part_cli.hpp"
#include "server_reconnect_cli.hpp"
#include "server_topic_cli.hpp"
#include "watch_cli.hpp"

#include "alias.hpp"
#include "cli.hpp"

using boost::format;
using boost::str;

using boost::asio::ip::tcp;

#if !BOOST_OS_WINDOWS

using boost::asio::local::stream_protocol;

#endif

namespace irccd {

namespace ctl {

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

/*
 * Configuration file parsing.
 * -------------------------------------------------------------------
 */

void usage()
{
    std::exit(1);
}

/*
 * resolve
 * ------------------------------------------------------------------
 *
 * Block unless host/port has been resolved.
 */
std::vector<tcp::endpoint> resolve(const std::string& host, const std::string& name)
{
    std::vector<tcp::endpoint> endpoints;

    tcp::resolver resolver(service);
    tcp::resolver::query query(host, name);

    for (auto it = resolver.resolve(query); it != tcp::resolver::iterator(); ++it)
        endpoints.push_back(*it);

    return endpoints;
}

/*
 * read_connect_ip
 * -------------------------------------------------------------------
 *
 * Extract IP connection information from the config file.
 *
 * [connect]
 * type = "ip"
 * host = "ip or hostname"
 * port = "port number or service"
 * domain = "ipv4 or ipv6" (Optional, default: ipv4)
 * ssl = true | false
 */
std::unique_ptr<io::connector> read_connect_ip(const ini::section& sc)
{
    const auto host = sc.get("host").value();
    const auto port = sc.get("port").value();

    if (host.empty())
        throw transport_error(transport_error::invalid_hostname);
    if (port.empty())
        throw transport_error(transport_error::invalid_port);

    const auto endpoints = resolve(host, port);

    if (string_util::is_boolean(sc.get("ssl").value())) {
#if defined(IRCCD_HAVE_SSL)
        // TODO: support more parameters.
        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

        return std::make_unique<io::tls_connector<>>(std::move(ctx), service, endpoints);
#else
        throw std::runtime_error("SSL disabled");
#endif
    }

    return std::make_unique<io::ip_connector>(service, endpoints);
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
std::unique_ptr<io::connector> read_connect_local(const ini::section& sc)
{
#if !BOOST_OS_WINDOWS
    using boost::asio::local::stream_protocol;

    const auto it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("missing path parameter");

    return std::make_unique<io::local_connector>(service, it->value());
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

    std::unique_ptr<io::connector> connector;

    if (it->value() == "ip")
        connector = read_connect_ip(sc);
    else if (it->value() == "unix")
        connector = read_connect_local(sc);
    else
        throw std::invalid_argument(str(format("invalid type given: %1%") % it->value()));

    if (connector) {
        ctl = std::make_unique<controller>(std::move(connector));

        auto password = sc.find("password");

        if (password != sc.end())
            ctl->set_password(password->value());
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
        verbose = string_util::is_boolean(value->value());
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
alias read_alias(const ini::section& sc, const std::string& name)
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
                % name % option.key()));

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
        if (sc.key().compare(0, 6, "alias.") == 0) {
            auto name = sc.key().substr(6);
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
 * -t ip | ipv6
 * -h host or ip
 * -p port
 */
std::unique_ptr<io::connector> parse_connect_ip(const option::result& options)
{
    option::result::const_iterator it;

    // Host (-h or --host).
    if ((it = options.find("-h")) == options.end() && (it = options.find("--host")) == options.end())
        throw transport_error(transport_error::invalid_hostname);

    const auto host = it->second;

    // Port (-p or --port).
    if ((it = options.find("-p")) == options.end() && (it = options.find("--port")) == options.end())
        throw transport_error(transport_error::invalid_port);

    const auto port = it->second;

    return std::make_unique<io::ip_connector>(service, resolve(host, port));
}

/*
 * parse_connect_local
 * ------------------------------------------------------------------
 *
 * Parse local connection.
 *
 * -P file
 */
std::unique_ptr<io::connector> parse_connect_local(const option::result& options)
{
#if !BOOST_OS_WINDOWS
    option::result::const_iterator it;

    if ((it = options.find("-P")) == options.end() && (it = options.find("--path")) == options.end())
        throw std::invalid_argument("missing path parameter (-P or --path)");

    return std::make_unique<io::local_connector>(service, it->second);
#else
    (void)options;

    throw std::invalid_argument("unix connection not supported on Windows");
#endif
}

/*
 * parse_connect
 * ------------------------------------------------------------------
 *
 * Generic parsing of command line option for connection.
 */
void parse_connect(const option::result& options)
{
    assert(options.count("-t") > 0 || options.count("--type") > 0);

    auto it = options.find("-t");

    if (it == options.end())
        it = options.find("--type");

    std::unique_ptr<io::connector> connector;

    if (it->second == "ip" || it->second == "ipv6")
        connector = parse_connect_ip(options);
    if (it->second == "unix")
        connector = parse_connect_local(options);
    else
        throw std::invalid_argument(str(format("invalid type given: %1%") % it->second));

    if (connector)
        ctl = std::make_unique<controller>(std::move(connector));
}

option::result parse(int& argc, char**& argv)
{
    // 1. Parse command line options.
    option::options def{
        { "-c",         true    },
        { "--config",   true    },
        { "-h",         true    },
        { "--help",     false   },
        { "--host",     true    },
        { "-p",         true    },
        { "--port",     true    },
        { "-P",         true    },
        { "--path",     true    },
        { "-t",         true    },
        { "--type",     true    },
        { "-v",         false   },
        { "--verbose",  false   }
    };

    option::result result;

    try {
        result = option::read(argc, argv, def);

        if (result.count("--help") > 0 || result.count("-h") > 0)
            usage();
            // NOTREACHED

        if (result.count("-v") != 0 || result.count("--verbose") != 0)
            verbose = true;
    } catch (const std::exception& ex) {
        std::cerr << "irccdctl: " << ex.what() << std::endl;
        usage();
    }

    return result;
}

void exec(std::vector<std::string>);

void exec(const alias& alias, std::vector<std::string> args_copy)
{
    for (const auto& cmd : alias) {
        std::vector<std::string> args(args_copy);
        std::vector<std::string> cmd_args;
        std::vector<std::string>::size_type toremove = 0;

        // 1. Append command name before.
        cmd_args.push_back(cmd.command());

        for (const auto& arg : cmd.args()) {
            if (arg.is_placeholder()) {
                if (args.size() < arg.index() + 1)
                    throw std::invalid_argument(
                        str(format("missing argument for placeholder %1%") % arg.index()));

                cmd_args.push_back(args[arg.index()]);

                if (arg.index() + 1 > toremove)
                    toremove = arg.index() + 1;
            } else
                cmd_args.push_back(arg.value());
        }

        assert(toremove <= args.size());

        // 2. Remove the arguments that been placed in placeholders.
        args.erase(args.begin(), args.begin() + toremove);

        // 3. Now append the rest of arguments.
        std::copy(args.begin(), args.end(), std::back_inserter(cmd_args));

        // 4. Finally try to execute.
        exec(cmd_args);
    }
}

void exec(std::vector<std::string> args)
{
    assert(args.size() > 0);

    auto name = args[0];
    auto alias = aliases.find(name);

    // Remove name.
    args.erase(args.begin());

    if (alias != aliases.end())
        exec(alias->second, args);
    else {
        auto cmd = commands.find(name);

        if (cmd != commands.end())
            cmd->second->exec(*ctl, args);
        else
            throw std::invalid_argument("no alias or command named " + name);
    }
}

void init(int &argc, char **&argv)
{
    sys::set_program_name("irccdctl");

    -- argc;
    ++ argv;

    auto add = [] (auto c) {
        commands.emplace(c->name(), std::move(c));
    };

    add(std::make_unique<plugin_config_cli>());
    add(std::make_unique<plugin_info_cli>());
    add(std::make_unique<plugin_list_cli>());
    add(std::make_unique<plugin_load_cli>());
    add(std::make_unique<plugin_reload_cli>());
    add(std::make_unique<plugin_unload_cli>());
    add(std::make_unique<server_connect_cli>());
    add(std::make_unique<server_disconnect_cli>());
    add(std::make_unique<server_info_cli>());
    add(std::make_unique<server_invite_cli>());
    add(std::make_unique<server_join_cli>());
    add(std::make_unique<server_kick_cli>());
    add(std::make_unique<server_list_cli>());
    add(std::make_unique<server_me_cli>());
    add(std::make_unique<server_message_cli>());
    add(std::make_unique<server_mode_cli>());
    add(std::make_unique<server_nick_cli>());
    add(std::make_unique<server_notice_cli>());
    add(std::make_unique<server_part_cli>());
    add(std::make_unique<server_reconnect_cli>());
    add(std::make_unique<server_topic_cli>());
    add(std::make_unique<rule_add_cli>());
    add(std::make_unique<rule_edit_cli>());
    add(std::make_unique<rule_list_cli>());
    add(std::make_unique<rule_info_cli>());
    add(std::make_unique<rule_move_cli>());
    add(std::make_unique<rule_remove_cli>());
    add(std::make_unique<watch_cli>());
}

void do_connect()
{
    ctl->connect([&] (auto code, auto info) {
        if (code)
            throw std::system_error(code);

        if (verbose) {
            const json_util::document doc(info);
            const auto major = doc.get<int>("/major");
            const auto minor = doc.get<int>("/minor");
            const auto patch = doc.get<int>("/patch");

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

void do_exec(int argc, char** argv)
{
    std::vector<std::string> args;

    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    exec(args);
    service.run();
}

} // !namespace

} // !ctl

} // !irccd

int main(int argc, char** argv)
{
    irccd::ctl::init(argc, argv);

    // 1. Read command line arguments.
    auto result = irccd::ctl::parse(argc, argv);

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
        if (result.count("-t") > 0 || result.count("--type") > 0)
            irccd::ctl::parse_connect(result);

        auto it = result.find("-c");

        if (it != result.end() || (it = result.find("--config")) != result.end())
            irccd::ctl::read(it->second);
        else {
            if (auto conf = irccd::config::search("irccdctl.conf"))
                irccd::ctl::read(*conf);
        }
    } catch (const std::exception& ex) {
        std::cerr << "abort: " << ex.what() << std::endl;
        return 1;
    }

    if (argc <= 0)
        irccd::ctl::usage();
        // NOTREACHED

    if (!irccd::ctl::ctl) {
        std::cerr << "abort: no connection specified" << std::endl;
        return 1;
    }

    try {
        irccd::ctl::do_connect();
        irccd::ctl::do_exec(argc, argv);
    } catch (const std::system_error& ex) {
        std::cerr << "abort: " << ex.code().message() << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "abort: " << ex.what() << std::endl;
        return 1;
    }
}
