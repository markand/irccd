/*
 * irccdctl.cpp -- main irccdctl class
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
#include <cassert>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>

#include <format.h>

#include "command.hpp"
#include "elapsed-timer.hpp"
#include "fs.hpp"
#include "ini.hpp"
#include "irccdctl.hpp"
#include "json.hpp"
#include "net.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "path.hpp"
#include "sysconfig.hpp"
#include "system.hpp"
#include "util.hpp"

namespace irccd {

using namespace fmt::literals;

using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace std::string_literals;

/*
 * Config file format
 * ------------------------------------------------------------------
 *
 * [connect]
 * type = "ip | unix"
 *
 * # if ip
 * host = ""
 * port = number
 * domain = "ipv4 | ipv6", default: ipv4
 *
 * # if unix
 * path = ""
 *
 * [alias]
 * name = replacement
 */

/*
 * Initialize a connection from the configuration file
 * ------------------------------------------------------------------
 */

void Irccdctl::usage() const
{
    // TODO: CHANGE
    log::warning() << "usage: " << sys::programName() << " [options...] <command> [command-options...] [command-args...]\n\n";
    log::warning() << "General options:\n";
    log::warning() << "\tc, --config file\tspecify the configuration file\n";
    log::warning() << "\t--help\t\t\tshow this help\n";
    log::warning() << "\t-t, --type type\t\tspecify connection type\n";
    log::warning() << "\t-v, --verbose\t\tbe verbose\n\n";
    log::warning() << "Available options for type ip and ipv6 (-t, --type):\n";
    log::warning() << "\t-h, --host address\tconnect to the specified address\n";
    log::warning() << "\t-p, --port port\t\tuse the specified port number\n\n";
    log::warning() << "Available options for type unix (-t, --type):\n";
    log::warning() << "\t-P, --path file\t\tconnect to the specified socket file\n\n";
    log::warning() << "General commands:\n";
    log::warning() << "\thelp\t\t\tShow an help topic\n";
    log::warning() << "\twatch\t\t\tStart listening to irccd\n\n";
    log::warning() << "Plugin management:\n";
    log::warning() << "\tplugin-config\t\tGet or set a plugin configuration option\n";
    log::warning() << "\tplugin-info\t\tGet plugin information\n";
    log::warning() << "\tplugin-list\t\tList all loaded plugins\n";
    log::warning() << "\tplugin-load\t\tLoad a plugin\n";
    log::warning() << "\tplugin-reload\t\tReload a plugin\n";
    log::warning() << "\tplugin-unload\t\tUnload a plugin\n\n";
    log::warning() << "Server management:\n";
    log::warning() << "\tserver-cmode\t\tChange a channel mode\n";
    log::warning() << "\tserver-cnotice\t\tSend a channel notice\n";
    log::warning() << "\tserver-connect\t\tConnect to a server\n";
    log::warning() << "\tserver-disconnect\tDisconnect from a server\n";
    log::warning() << "\tserver-info\t\tGet server information\n";
    log::warning() << "\tserver-invite\t\tInvite someone to a channel\n";
    log::warning() << "\tserver-join\t\tJoin a channel\n";
    log::warning() << "\tserver-kick\t\tKick someone from a channel\n";
    log::warning() << "\tserver-list\t\tList all servers\n";
    log::warning() << "\tserver-me\t\tSend a CTCP Action (same as /me)\n";
    log::warning() << "\tserver-message\t\tSend a message to someone or a channel\n";
    log::warning() << "\tserver-mode\t\tChange a user mode\n";
    log::warning() << "\tserver-notice\t\tSend a private notice\n";
    log::warning() << "\tserver-nick\t\tChange your nickname\n";
    log::warning() << "\tserver-part\t\tLeave a channel\n";
    log::warning() << "\tserver-reconnect\tReconnect one or all servers\n";
    log::warning() << "\tserver-topic\t\tChange a channel topic\n";
    log::warning() << "\nFor more information on a command, type " << sys::programName() << " help <command>" << std::endl;
    std::exit(1);
}

void Irccdctl::readConnectIp(const ini::Section &sc)
{
    ini::Section::const_iterator it;

    // Host.
    std::string host;

    if ((it = sc.find("host")) == sc.end())
        throw std::invalid_argument("missing host parameter");

    host = it->value();

    // Port.
    std::uint16_t port;

    if ((it = sc.find("port")) == sc.end())
        throw std::invalid_argument("missing port parameter");

    try {
        port = util::toNumber<std::uint16_t>(it->value());
    } catch (...) {
        throw std::invalid_argument("invalid port number: " + it->value());
    }

    // Domain.
    int domain = AF_INET;

    if ((it = sc.find("domain")) != sc.end()) {
        if (it->value() == "ipv6")
            domain = AF_INET6;
        else if (it->value() == "ipv4")
            domain = AF_INET;
        else
            throw std::invalid_argument("invalid domain: " + it->value());
    }

    m_connection =  std::make_unique<ConnectionBase<net::address::Ip>>(net::address::Ip::resolve(host, std::to_string(port), domain));
}

void Irccdctl::readConnectUnix(const ini::Section &sc)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    auto it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("missing path parameter");

    m_connection = std::make_unique<ConnectionBase<net::address::Local>>(net::address::Local(it->value(), false));
#else
    (void)sc;

    throw std::invalid_argument("unix connection not supported on Windows");
#endif
}

void Irccdctl::readConnect(const ini::Section &sc)
{
    auto it = sc.find("type");

    if (it == sc.end())
        throw std::invalid_argument("missing type parameter");

    if (it->value() == "ip")
        readConnectIp(sc);
    else if (it->value() == "unix")
        readConnectUnix(sc);
    else
        throw std::invalid_argument("invalid type given: " + it->value());
}

void Irccdctl::readGeneral(const ini::Section &sc)
{
    auto verbose = sc.find("verbose");

    if (verbose != sc.end())
        log::setVerbose(util::isBoolean(verbose->value()));
}

void Irccdctl::readAliases(const ini::Section &sc)
{
    for (const ini::Option &option : sc) {
        // This is the alias name.
        Alias alias(option.key());

        if (m_commandService.contains(option.key()))
            throw std::invalid_argument("there is already a command named " + option.key());

        // Iterate over the list of commands to execute for this alias.
        for (const std::string &repl : option) {
            // This is the alias split string.
            std::vector<std::string> list = util::split(repl, " \t");

            if (list.size() < 1)
                throw std::invalid_argument("alias require at least one argument");

            // First argument is the command/alias to execute.
            std::string command = list[0];

            // This is the alias arguments.
            std::vector<AliasArg> args;

            for (auto it = list.begin() + 1; it != list.end(); ++it)
                args.push_back(std::move(*it));

            alias.push_back({std::move(command), std::move(args)});
        }

        // Show for debugging purpose.
        log::debug("alias {}:"_format(option.key()));

        for (const auto &cmd : alias)
            log::debug("  {} {}"_format(cmd.command(), util::join(cmd.args().begin(), cmd.args().end(), ' ')));

        m_aliases.emplace(option.key(), std::move(alias));
    }
}

void Irccdctl::read(const std::string &path, const option::Result &options)
{
    ini::Document doc = ini::readFile(path);
    ini::Document::const_iterator it = doc.find("connect");

    // Do not try to read [connect] if specified at command line.
    if (it != doc.end() && options.count("-t") == 0 && options.count("--type") == 0)
        readConnect(*it);
    if ((it = doc.find("general")) != doc.end())
        readGeneral(*it);
    if ((it = doc.find("alias")) != doc.end())
        readAliases(*it);
}

/*
 * Initialize a connection from the command line.
 * ------------------------------------------------------------------
 */

void Irccdctl::parseConnectIp(const option::Result &options, bool ipv6)
{
    option::Result::const_iterator it;

    // Host (-h or --host).
    std::string host;

    if ((it = options.find("-h")) == options.end() && (it = options.find("--host")) == options.end())
        throw std::invalid_argument("missing host argument (-h or --host)");

    host = it->second;

    // Port (-p or --port).
    std::uint16_t port;

    if ((it = options.find("-p")) == options.end() && (it = options.find("--port")) == options.end())
        throw std::invalid_argument("missing port argument (-p or --port)");

    try {
        port = util::toNumber<std::uint16_t>(it->second);
    } catch (...) {
        throw std::invalid_argument("invalid port number: " + it->second);
    }

    // Domain
    int domain = (ipv6) ? AF_INET6 : AF_INET;

    m_connection =  std::make_unique<ConnectionBase<net::address::Ip>>(net::address::Ip::resolve(host, std::to_string(port), domain, SOCK_STREAM));
}

void Irccdctl::parseConnectUnix(const option::Result &options)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    option::Result::const_iterator it;

    if ((it = options.find("-P")) == options.end() && (it = options.find("--path")) == options.end())
        throw std::invalid_argument("missing path parameter (-P or --path)");

    m_connection = std::make_unique<ConnectionBase<net::address::Local>>(net::address::Local(it->second, false));
#else
    (void)options;

    throw std::invalid_argument("unix connection not supported on Windows");
#endif
}

void Irccdctl::parseConnect(const option::Result &options)
{
    assert(options.count("-t") > 0 || options.count("--type") > 0);

    auto it = options.find("-t");

    if (it == options.end())
        it = options.find("--type");
    if (it->second == "ip" || it->second == "ipv6")
        return parseConnectIp(options, it->second == "ipv6");
    if (it->second == "unix")
        return parseConnectUnix(options);

    throw std::invalid_argument("invalid type given: " + it->second);
}

option::Result Irccdctl::parse(int &argc, char **&argv) const
{
    // 1. Parse command line options.
    option::Options def{
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

    option::Result result;

    try {
        result = option::read(argc, argv, def);

        if (result.count("--help") != 0) {
            usage();
            // NOTREACHED
        }

        if (result.count("-v") != 0 || result.count("--verbose") != 0)
            log::setVerbose(true);
    } catch (const std::exception &ex) {
        log::warning("{}: {}"_format(sys::programName(), ex.what()));
        usage();
    }

    return result;
}

void Irccdctl::exec(const Command &cmd, std::vector<std::string> args)
{
    // 1. Build options from command line arguments.
    option::Options def;

    for (const auto &opt : cmd.options()) {
        // parser::read needs '-' and '--' so add them.
        if (!opt.simpleKey().empty())
            def.emplace("-"s + opt.simpleKey(), !opt.arg().empty());
        if (!opt.longKey().empty())
            def.emplace("--"s + opt.longKey(), !opt.arg().empty());
    }

    // 2. Parse them, remove them from args (in parser::read) and build the map with id.
    CommandRequest::Options requestOptions;

    for (const auto &pair : option::read(args, def)) {
        auto options = cmd.options();
        auto it = std::find_if(options.begin(), options.end(), [&] (const auto &opt) {
            return ("-"s + opt.simpleKey()) == pair.first || ("--"s + opt.longKey()) == pair.first;
        });

        requestOptions.emplace(it->id(), pair.second);
    }

    // 3. Check number of arguments.
    if (args.size() < cmd.min())
        throw std::runtime_error("too few arguments");
    if (args.size() > cmd.max())
        throw std::runtime_error("too many arguments");

    // 4. Construct the request, if the returned value is not an object, do not send anything (e.g. help).
    nlohmann::json request = cmd.request(*this, CommandRequest(std::move(requestOptions), std::move(args)));

    if (!request.is_object())
        return;

    request.push_back({"command", cmd.name()});

    // 5. Send the command.
    m_connection->send(request.dump(), 30000);

    // 6. Parse the result.
    cmd.result(*this, m_connection->next(cmd.name(), 30000));
}

void Irccdctl::exec(const Alias &alias, std::vector<std::string> argsCopy)
{
    for (const AliasCommand &cmd : alias) {
        std::vector<std::string> args(argsCopy);
        std::vector<std::string> cmdArgs;
        std::vector<std::string>::size_type toremove = 0;

        // 1. Append command name before.
        cmdArgs.push_back(cmd.command());

        for (const AliasArg &arg : cmd.args()) {
            if (arg.isPlaceholder()) {
                if (args.size() < arg.index() + 1)
                    throw std::invalid_argument("missing argument for placeholder %" + std::to_string(arg.index()));

                cmdArgs.push_back(args[arg.index()]);

                if (arg.index() + 1 > toremove)
                    toremove = arg.index() + 1;
            } else
                cmdArgs.push_back(arg.value());
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

void Irccdctl::exec(std::vector<std::string> args)
{
    assert(args.size() > 0);

    auto name = args[0];
    auto alias = m_aliases.find(name);

    // Remove name.
    args.erase(args.begin());

    if (alias != m_aliases.end())
        exec(alias->second, args);
    else {
        auto cmd = m_commandService.find(name);

        if (cmd)
            exec(*cmd, args);
        else
            throw std::invalid_argument("no alias or command named " + name);
    }
}

void Irccdctl::connect()
{
    log::info("{}: connecting to irccd..."_format(sys::programName()));

    // Try to connect.
    m_connection->connect(30000);

    // Get irccd information.
    auto object = m_connection->next(30000);
    auto program = object.find("program");

    if (program == object.end() || !program->is_string() || program->get<std::string>() != "irccd")
        throw std::runtime_error("not an irccd server");

    // Get values.
    m_major = util::json::toInt(object["major"]);
    m_minor = util::json::toInt(object["minor"]);
    m_patch = util::json::toInt(object["patch"]);
    m_javascript = util::json::toBool(object["javascript"]);
    m_ssl = util::json::toBool(object["ssl"]);

    log::info() << std::boolalpha;
    log::info("{}: connected to irccd {}.{}.{}"_format(sys::programName(), m_major, m_minor, m_patch));
    log::info("{}: javascript: {}, ssl support: {}"_format(sys::programName(), m_javascript, m_ssl));
}

void Irccdctl::run(int argc, char **argv)
{
    // 1. Read command line arguments.
    option::Result result = parse(argc, argv);

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
            read(it->second, result);
        else {
            for (const std::string &dir : path::list(path::PathConfig)) {
                std::string path = dir + "irccdctl.conf";

                if (fs::exists(path))
                    read(path, result);
            }
        }
    } catch (const std::exception &ex) {
        log::warning("{}: {}"_format(sys::programName(), ex.what()));
        std::exit(1);
    }

    if (argc <= 0) {
        usage();
        // NOTREACHED
    }

    // Help does not require connection.
    if (std::strcmp(argv[0], "help") != 0) {
        if (!m_connection) {
            log::warning("{}: no connection specified"_format(sys::programName()));
            std::exit(1);
        }

        connect();
    }

    // Build a vector of arguments.
    std::vector<std::string> args;

    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    exec(args);
}

} // !irccd
