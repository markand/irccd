/*
 * main.cpp -- irccd controller main
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

#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>

#include "alias.hpp"
#include "cli.hpp"
#include "client.hpp"
#include "ini.hpp"
#include "irccdctl.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "system.hpp"
#include "string_util.hpp"
#include "util.hpp"

using namespace std::string_literals;
using namespace irccd;

namespace {

std::vector<std::unique_ptr<Cli>> commands;
std::unordered_map<std::string, Alias> aliases;
std::unique_ptr<Client> client;
std::unique_ptr<Irccdctl> irccdctl;
net::Address address;

void usage()
{
    bool first = true;

    for (const auto &cmd : commands) {
        log::warning() << (first ? "usage: " : "       ") << sys::program_name() << " "
                       << cmd->usage() << std::endl;
        first = false;
    }

    std::exit(1);
}

void help()
{
    log::warning() << "usage: " << sys::program_name() << " [options...] <command> [command-options...] [command-args...]\n\n";
    log::warning() << "General options:\n";
    log::warning() << "\t-c, --config file\tspecify the configuration file\n";
    log::warning() << "\t    --help\t\tshow this help\n";
    log::warning() << "\t-t, --type type\t\tspecify connection type\n";
    log::warning() << "\t-v, --verbose\t\tbe verbose\n\n";
    log::warning() << "Available options for type ip and ipv6 (-t, --type):\n";
    log::warning() << "\t-h, --host address\tconnect to the specified address\n";
    log::warning() << "\t-p, --port port\t\tuse the specified port number\n\n";
    log::warning() << "Available options for type unix (-t, --type):\n";
    log::warning() << "\t-P, --path file\t\tconnect to the specified socket file\n\n";
    log::warning() << "Available commands:\n";

    for (const auto &cmd : commands)
        log::warning() << "\t" << std::left << std::setw(32)
                       << cmd->name() << cmd->summary() << std::endl;

    log::warning() << "\nFor more information on a command, type " << sys::program_name() << " help <command>" << std::endl;
    std::exit(1);
}

void help(const std::string &command)
{
    auto it = std::find_if(commands.begin(), commands.end(), [&] (const auto &c) {
        return c->name() == command;
    });

    if (it == commands.end()) {
        log::warning() << "no command named " << command << std::endl;
    } else {
        log::warning() << "usage: " << sys::program_name() << " " << (*it)->usage() << "\n" << std::endl;
        log::warning() << (*it)->help() << std::endl;
    }

    std::exit(1);
}

/*
 * Configuration file parsing.
 * -------------------------------------------------------------------
 */

/*
 * readConnectIp
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
void readConnectIp(const ini::section &sc)
{
    ini::section::const_iterator it;
    std::string host, port;

    if ((it = sc.find("host")) == sc.end())
        throw std::invalid_argument("missing host parameter");

    host = it->value();

    if ((it = sc.find("port")) == sc.end())
        throw std::invalid_argument("missing port parameter");

    port = it->value();

    int domain = AF_INET;

    if ((it = sc.find("domain")) != sc.end() || (it = sc.find("family")) != sc.end()) {
        if (it->value() == "ipv6") {
            domain = AF_INET6;
        } else if (it->value() == "ipv4") {
            domain = AF_INET;
        } else {
            throw std::invalid_argument("invalid family: " + it->value());
        }
    }

    address = net::resolveOne(host, port, domain, SOCK_STREAM);

    if ((it = sc.find("ssl")) != sc.end() && string_util::is_boolean(it->value()))
#if defined(HAVE_SSL)
        client = std::make_unique<TlsClient>();
#else
        throw std::runtime_error("SSL disabled");
#endif
    else
        client = std::make_unique<Client>();
}

/*
 * readConnectLocal
 * -------------------------------------------------------------------
 *
 * Extract local connection for Unix.
 *
 * [connect]
 * type = "unix"
 * path = "path to socket file"
 */
void readConnectLocal(const ini::section &sc)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    auto it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("missing path parameter");

    address = net::local::create(it->value());
    client = std::make_unique<Client>();
#else
    (void)sc;

    throw std::invalid_argument("unix connection not supported on Windows");
#endif
}

/*
 * readConnect
 * -------------------------------------------------------------------
 *
 * Generic function for reading the [connect] section.
 */
void readConnect(const ini::section &sc)
{
    auto it = sc.find("type");

    if (it == sc.end())
        throw std::invalid_argument("missing type parameter");

    if (it->value() == "ip") {
        readConnectIp(sc);
    } else if (it->value() == "unix") {
        readConnectLocal(sc);
    } else {
        throw std::invalid_argument("invalid type given: " + it->value());
    }

    auto password = sc.find("password");

    if (password != sc.end())
        client->setPassword(password->value());
}

/*
 * readGeneral
 * -------------------------------------------------------------------
 *
 * Read the general section.
 *
 * [general]
 * verbose = true
 */
void readGeneral(const ini::section &sc)
{
    auto verbose = sc.find("verbose");

    if (verbose != sc.end())
        log::set_verbose(string_util::is_boolean(verbose->value()));
}

/*
 * readAlias
 * -------------------------------------------------------------------
 *
 * Read aliases for irccdctl.
 *
 * [alias.<name>]
 * cmd1 = ( "command", "arg1, "...", "argn" )
 * cmd2 = ( "command", "arg1, "...", "argn" )
 */
Alias readAlias(const ini::section &sc, const std::string &name)
{
    Alias alias(name);

    /*
     * Each defined option is a command that the user can call. The name is
     * unused and serves as documentation purpose.
     */
    for (const auto &option : sc) {
        /*
         * Iterate over the arguments which are usually a list and the first
         * argument is a command name.
         */
        if (option.size() == 1 && option[0].empty())
            throw std::runtime_error(string_util::sprintf("alias %s: missing command name in '%s'", name, option.key()));

        std::string command = option[0];
        std::vector<AliasArg> args(option.begin() + 1, option.end());

        alias.emplace_back(std::move(command), std::move(args));
    }

    return alias;
}

void read(const std::string &path)
{
    try {
        ini::document doc = ini::read_file(path);
        ini::document::const_iterator it;

        if (!client && (it = doc.find("connect")) != doc.end())
            readConnect(*it);
        if ((it = doc.find("general")) != doc.end())
            readGeneral(*it);

        // [alias.*] sections.
        for (const auto& sc : doc) {
            if (sc.key().compare(0, 6, "alias.") == 0) {
                auto name = sc.key().substr(6);
                auto alias = readAlias(sc, name);

                aliases.emplace(std::move(name), std::move(alias));
            }
        }
    } catch (const std::exception &ex) {
        log::warning() << path << ": " << ex.what() << std::endl;
    }
}

/*
 * Command line parsing.
 * -------------------------------------------------------------------
 */

/*
 * parseConnectIp
 * ------------------------------------------------------------------
 *
 * Parse internet connection from command line.
 *
 * -t ip | ipv6
 * -h host or ip
 * -p port
 */
void parseConnectIp(const option::result &options)
{
    option::result::const_iterator it;

    // Host (-h or --host).
    std::string host;

    if ((it = options.find("-h")) == options.end() && (it = options.find("--host")) == options.end()) {
        throw std::invalid_argument("missing host argument (-h or --host)");
    }

    host = it->second;

    // Port (-p or --port).
    std::string port;

    if ((it = options.find("-p")) == options.end() && (it = options.find("--port")) == options.end()) {
        throw std::invalid_argument("missing port argument (-p or --port)");
    }

    port = it->second;

    // Domain
    int domain = AF_INET;

    if ((it = options.find("-t")) != options.end()) {
        domain = it->second == "ipv6" ? AF_INET6 : AF_INET;
    } else if ((it = options.find("--type")) != options.end()) {
        domain = it->second == "ipv6" ? AF_INET6: AF_INET;
    }

    address = net::resolveOne(host, port, domain, SOCK_STREAM);
    client  = std::make_unique<Client>();
}

/*
 * parseConnectLocal
 * ------------------------------------------------------------------
 *
 * Parse local connection.
 *
 * -P file
 */
void parseConnectLocal(const option::result &options)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    option::result::const_iterator it;

    if ((it = options.find("-P")) == options.end() && (it = options.find("--path")) == options.end())
        throw std::invalid_argument("missing path parameter (-P or --path)");

    address = net::local::create(it->second, false);
    client = std::make_unique<Client>();
#else
    (void)options;

    throw std::invalid_argument("unix connection not supported on Windows");
#endif
}

/*
 * parseConnect
 * ------------------------------------------------------------------
 *
 * Generic parsing of command line option for connection.
 */
void parseConnect(const option::result &options)
{
    assert(options.count("-t") > 0 || options.count("--type") > 0);

    auto it = options.find("-t");

    if (it == options.end())
        it = options.find("--type");
    if (it->second == "ip" || it->second == "ipv6")
        return parseConnectIp(options);
    if (it->second == "unix")
        return parseConnectLocal(options);

    throw std::invalid_argument("invalid type given: " + it->second);
}

option::result parse(int &argc, char **&argv)
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

        if (result.count("--help") != 0) {
            usage();
            // NOTREACHED
        }

        if (result.count("-v") != 0 || result.count("--verbose") != 0) {
            log::set_verbose(true);
        }
    } catch (const std::exception &ex) {
        log::warning() << "irccdctl: " << ex.what() << std::endl;
        usage();
    }

    return result;
}

void exec(std::vector<std::string>);

void exec(const Alias &alias, std::vector<std::string> argsCopy)
{
    std::vector<nlohmann::json> values;

    for (const AliasCommand &cmd : alias) {
        std::vector<std::string> args(argsCopy);
        std::vector<std::string> cmdArgs;
        std::vector<std::string>::size_type toremove = 0;

        // 1. Append command name before.
        cmdArgs.push_back(cmd.command());

        for (const auto &arg : cmd.args()) {
            if (arg.isPlaceholder()) {
                if (args.size() < arg.index() + 1)
                    throw std::invalid_argument(string_util::sprintf("missing argument for placeholder %d", arg.index()));

                cmdArgs.push_back(args[arg.index()]);

                if (arg.index() + 1 > toremove) {
                    toremove = arg.index() + 1;
                }
            } else {
                cmdArgs.push_back(arg.value());
            }
        }

        assert(toremove <= args.size());

        // 2. Remove the arguments that been placed in placeholders.
        args.erase(args.begin(), args.begin() + toremove);

        // 3. Now append the rest of arguments.
        std::copy(args.begin(), args.end(), std::back_inserter(cmdArgs));

        // 4. Finally try to execute.
        exec(cmdArgs);
    }
}

void exec(std::vector<std::string> args)
{
    assert(args.size() > 0);

    auto name = args[0];
    auto alias = aliases.find(name);

    // Remove name.
    args.erase(args.begin());

    if (alias != aliases.end()) {
        exec(alias->second, args);
    } else {
        auto cmd = std::find_if(commands.begin(), commands.end(), [&] (auto &it) {
            return it->name() == name;
        });

        if (cmd != commands.end()) {
            (*cmd)->exec(*irccdctl, args);
        } else {
            throw std::invalid_argument("no alias or command named " + name);
        }
    }
}

void init(int &argc, char **&argv)
{
    sys::set_program_name("irccdctl");
    net::init();

    --argc;
    ++argv;

    commands.push_back(std::make_unique<cli::PluginConfigCli>());
    commands.push_back(std::make_unique<cli::PluginInfoCli>());
    commands.push_back(std::make_unique<cli::PluginListCli>());
    commands.push_back(std::make_unique<cli::PluginLoadCli>());
    commands.push_back(std::make_unique<cli::PluginReloadCli>());
    commands.push_back(std::make_unique<cli::PluginUnloadCli>());
    commands.push_back(std::make_unique<cli::ServerChannelMode>());
    commands.push_back(std::make_unique<cli::ServerChannelNoticeCli>());
    commands.push_back(std::make_unique<cli::ServerConnectCli>());
    commands.push_back(std::make_unique<cli::ServerDisconnectCli>());
    commands.push_back(std::make_unique<cli::ServerInfoCli>());
    commands.push_back(std::make_unique<cli::ServerInviteCli>());
    commands.push_back(std::make_unique<cli::ServerJoinCli>());
    commands.push_back(std::make_unique<cli::ServerKickCli>());
    commands.push_back(std::make_unique<cli::ServerListCli>());
    commands.push_back(std::make_unique<cli::ServerMeCli>());
    commands.push_back(std::make_unique<cli::ServerMessageCli>());
    commands.push_back(std::make_unique<cli::ServerModeCli>());
    commands.push_back(std::make_unique<cli::ServerNickCli>());
    commands.push_back(std::make_unique<cli::ServerNoticeCli>());
    commands.push_back(std::make_unique<cli::ServerPartCli>());
    commands.push_back(std::make_unique<cli::ServerReconnectCli>());
    commands.push_back(std::make_unique<cli::ServerTopicCli>());
    commands.push_back(std::make_unique<cli::RuleAddCli>());
    commands.push_back(std::make_unique<cli::RuleEditCli>());
    commands.push_back(std::make_unique<cli::RuleListCli>());
    commands.push_back(std::make_unique<cli::RuleInfoCli>());
    commands.push_back(std::make_unique<cli::RuleMoveCli>());
    commands.push_back(std::make_unique<cli::RuleRemoveCli>());
    commands.push_back(std::make_unique<cli::WatchCli>());
}

} // !namespace

int main(int argc, char **argv)
{
    init(argc, argv);

    // 1. Read command line arguments.
    auto result = parse(argc, argv);

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
            parseConnect(result);

        auto it = result.find("-c");

        if (it != result.end() || (it = result.find("--config")) != result.end())
            read(it->second);
        else {
            for (const auto& path : sys::config_filenames("irccdctl.conf")) {
                boost::system::error_code ec;

                if (boost::filesystem::exists(path, ec) && !ec) {
                    read(path);
                    break;
                }
            }
        }
    } catch (const std::exception &ex) {
        log::warning() << sys::program_name() << ": " << ex.what() << std::endl;
        std::exit(1);
    }

    if (argc <= 0) {
        usage();
        // NOTREACHED
    }
    if (std::strcmp(argv[0], "help") == 0) {
        if (argc >= 2)
            help(argv[1]);
        else
            help();
        // NOTREACHED
    }

    if (!client) {
        log::warning("irccdctl: no connection specified");
        std::exit(1);
    }

    irccdctl = std::make_unique<Irccdctl>(std::move(client));
    irccdctl->client().onDisconnect.connect([&] (auto reason) {
        log::warning() << "connection lost to irccd: " << reason << std::endl;
    });
    irccdctl->client().onConnect.connect([&] (auto info) {
        log::info() << "connected to irccd "
                    << info.major << "."
                    << info.minor << "."
                    << info.patch << std::endl;
    });
    irccdctl->client().connect(address);

    // Build a vector of arguments.
    std::vector<std::string> args;

    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    try {
        exec(args);
    } catch (const std::exception &ex) {
        std::cerr << sys::program_name() << ": unrecoverable error: " << ex.what() << std::endl;
    }
}
