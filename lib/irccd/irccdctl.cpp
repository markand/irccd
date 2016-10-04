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

#include <format.h>

#include "command.hpp"
#include "client.hpp"
#include "elapsed-timer.hpp"
#include "fs.hpp"
#include "ini.hpp"
#include "irccdctl.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "path.hpp"
#include "system.hpp"
#include "util.hpp"

using namespace std::string_literals;

using namespace fmt::literals;

namespace irccd {

void Irccdctl::usage() const
{
    bool first = true;

    for (const auto &cmd : m_commandService.commands()) {
        log::warning() << (first ? "usage: " : "       ") << sys::programName() << " "
                       << cmd->usage() << std::endl;
        first = false;
    }

    std::exit(1);
}

void Irccdctl::help() const
{
    log::warning() << "usage: " << sys::programName() << " [options...] <command> [command-options...] [command-args...]\n\n";
    log::warning() << "General options:\n";
    log::warning() << "\t-c, --config file\tspecify the configuration file\n";
    log::warning() << "\t--help\t\t\tshow this help\n";
    log::warning() << "\t-t, --type type\t\tspecify connection type\n";
    log::warning() << "\t-v, --verbose\t\tbe verbose\n\n";
    log::warning() << "Available options for type ip and ipv6 (-t, --type):\n";
    log::warning() << "\t-h, --host address\tconnect to the specified address\n";
    log::warning() << "\t-p, --port port\t\tuse the specified port number\n\n";
    log::warning() << "Available options for type unix (-t, --type):\n";
    log::warning() << "\t-P, --path file\t\tconnect to the specified socket file\n\n";
    log::warning() << "Available commands:\n";

    for (const auto &cmd : m_commandService.commands())
        log::warning() << "\t" << std::left << std::setw(32)
                       << cmd->name() << cmd->description() << std::endl;

    log::warning() << "\nFor more information on a command, type " << sys::programName() << " help <command>" << std::endl;

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
void Irccdctl::readConnectIp(const ini::Section &sc)
{
    ini::Section::const_iterator it;

    std::string host, port;

    if ((it = sc.find("host")) == sc.end())
        throw std::invalid_argument("missing host parameter");

    host = it->value();

    if ((it = sc.find("port")) == sc.end())
        throw std::invalid_argument("missing port parameter");

    port = it->value();

    int domain = AF_INET;

    if ((it = sc.find("domain")) != sc.end()) {
        if (it->value() == "ipv6")
            domain = AF_INET6;
        else if (it->value() == "ipv4")
            domain = AF_INET;
        else
            throw std::invalid_argument("invalid domain: " + it->value());
    }

    m_address = net::resolveOne(host, port, domain, SOCK_STREAM);

    if ((it = sc.find("ssl")) != sc.end() && util::isBoolean(it->value()))
        m_connection = std::make_unique<TlsClient>();
    else
        m_connection = std::make_unique<Client>();
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
void Irccdctl::readConnectLocal(const ini::Section &sc)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    auto it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("missing path parameter");

    m_address = net::local::create(it->value());
    m_connection = std::make_unique<Client>();
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
void Irccdctl::readConnect(const ini::Section &sc)
{
    auto it = sc.find("type");

    if (it == sc.end())
        throw std::invalid_argument("missing type parameter");

    if (it->value() == "ip")
        readConnectIp(sc);
    else if (it->value() == "unix")
        readConnectLocal(sc);
    else
        throw std::invalid_argument("invalid type given: " + it->value());

    auto password = sc.find("password");

    if (password != sc.end())
        m_connection->setPassword(password->value());
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
void Irccdctl::readGeneral(const ini::Section &sc)
{
    auto verbose = sc.find("verbose");

    if (verbose != sc.end())
        log::setVerbose(util::isBoolean(verbose->value()));
}

/*
 * readAliases
 * -------------------------------------------------------------------
 *
 * Read aliases for irccdctl.
 *
 * [alias]
 * name = ( "command", "arg1, "...", "argn" )
 */
void Irccdctl::readAliases(const ini::Section &sc)
{
    for (const auto &option : sc) {
        // This is the alias name.
        Alias alias(option.key());

        // Iterate over the list of commands to execute for this alias.
        for (const auto &repl : option) {
            // This is the alias split string.
            auto list = util::split(repl, " \t");

            if (list.size() < 1)
                throw std::invalid_argument("alias require at least one argument");

            // First argument is the command/alias to execute.
            auto command = list[0];

            // Remove command name and puts arguments.
            alias.push_back({std::move(command), std::vector<AliasArg>(list.begin() + 1, list.end())});
        }

        m_aliases.emplace(option.key(), std::move(alias));
    }
}

void Irccdctl::read(const std::string &path)
{
    try {
        ini::Document doc = ini::readFile(path);
        ini::Document::const_iterator it;

        if (!m_connection && (it = doc.find("connect")) != doc.end())
            readConnect(*it);
        if ((it = doc.find("general")) != doc.end())
            readGeneral(*it);
        if ((it = doc.find("alias")) != doc.end())
            readAliases(*it);
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
void Irccdctl::parseConnectIp(const option::Result &options)
{
    option::Result::const_iterator it;

    // Host (-h or --host).
    std::string host;

    if ((it = options.find("-h")) == options.end() && (it = options.find("--host")) == options.end())
        throw std::invalid_argument("missing host argument (-h or --host)");

    host = it->second;

    // Port (-p or --port).
    std::string port;

    if ((it = options.find("-p")) == options.end() && (it = options.find("--port")) == options.end())
        throw std::invalid_argument("missing port argument (-p or --port)");

    port = it->second;

    // Domain
    int domain = AF_INET;

    if ((it = options.find("-t")) != options.end())
        domain = it->second == "ipv6" ? AF_INET6 : AF_INET;
    else if ((it = options.find("--type")) != options.end())
        domain = it->second == "ipv6" ? AF_INET6: AF_INET;

    m_address = net::resolveOne(host, port, domain, SOCK_STREAM);
    m_connection = std::make_unique<Client>();
}

/*
 * parseConnectLocal
 * ------------------------------------------------------------------
 *
 * Parse local connection.
 *
 * -P file
 */
void Irccdctl::parseConnectLocal(const option::Result &options)
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
    option::Result::const_iterator it;

    if ((it = options.find("-P")) == options.end() && (it = options.find("--path")) == options.end())
        throw std::invalid_argument("missing path parameter (-P or --path)");

    m_address = net::local::create(it->second, false);
    m_connection = std::make_unique<Client>();
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
void Irccdctl::parseConnect(const option::Result &options)
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

option::Result Irccdctl::parse(int &argc, char **&argv)
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

nlohmann::json Irccdctl::waitMessage(const std::string id)
{
    ElapsedTimer timer;

    while (m_messages.empty() && m_connection->isConnected() && timer.elapsed() < m_timeout)
        util::poller::poll(250, *m_connection);

    if (m_messages.empty())
        return nlohmann::json();

    nlohmann::json value;

    if (id == "") {
        value = m_messages[0];
        m_messages.erase(m_messages.begin());
    } else {
        auto it = std::find_if(m_messages.begin(), m_messages.end(), [&] (const auto &v) {
            auto rt = v.find("response");

            if (v.count("error") > 0 || (rt != v.end() && rt->is_string() && *rt == id))
                return true;

            return false;
        });

        // Remove the previous messages.
        if (it != m_messages.end()) {
            value = *it;
            m_messages.erase(m_messages.begin(), it + 1);
        }
    }

    auto error = value.find("error");

    if (error != value.end() && error->is_string())
        throw std::runtime_error(error->template get<std::string>());

    return value;
}

nlohmann::json Irccdctl::waitEvent()
{
    ElapsedTimer timer;

    while (m_events.empty() && m_connection->isConnected() && timer.elapsed() < m_timeout)
        util::poller::poll(250, *m_connection);

    if (m_events.empty())
        return nullptr;

    auto first = m_events.front();
    m_events.erase(m_events.begin());

    return first;
}

nlohmann::json Irccdctl::exec(const Command &cmd, std::vector<std::string> args)
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

    /*
     * 4. Construct the request, if the returned value is not an object, do not
     * send anything (e.g. help).
     */
    auto request = cmd.request(*this, CommandRequest(std::move(requestOptions), std::move(args)));

    if (!request.is_object())
        throw std::invalid_argument("command has returned invalid request");

    request.push_back({"command", cmd.name()});

    // 5. Send the command.
    m_connection->request(request);

    // 6. Returns the response.
    return waitMessage(cmd.name());
}

std::vector<nlohmann::json> Irccdctl::exec(const Alias &alias, std::vector<std::string> argsCopy)
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
        auto response = exec(cmdArgs);

        values.insert(values.end(), response.begin(), response.end());
    }

    return values;
}

std::vector<nlohmann::json> Irccdctl::exec(std::vector<std::string> args)
{
    assert(args.size() > 0);

    auto name = args[0];
    auto alias = m_aliases.find(name);

    // Remove name.
    args.erase(args.begin());

    std::vector<nlohmann::json> values;

    if (alias != m_aliases.end()) {
        auto response = exec(alias->second, args);

        values.insert(values.end(), response.begin(), response.end());
    } else {
        auto cmd = m_commandService.find(name);

        if (cmd)
            values.push_back(exec(*cmd, args));
        else
            throw std::invalid_argument("no alias or command named " + name);
    }

    return values;
}

void Irccdctl::run(int argc, char **argv)
{
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
            for (const std::string &dir : path::list(path::PathConfig)) {
                std::string path = dir + "irccdctl.conf";

                if (fs::exists(path)) {
                    read(path);
                    break;
                }
            }
        }
    } catch (const std::exception &ex) {
        log::warning() << sys::programName() << ": " << ex.what() << std::endl;
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

        m_connection->onDisconnect.connect([this] (auto reason) {
            log::warning() << "connection lost to irccd: " << reason << std::endl;
        });
        m_connection->onConnect.connect([this] (auto info) {
            log::info() << "connected to irccd "
                        << info.major << "."
                        << info.minor << "."
                        << info.patch << std::endl;
        });
        m_connection->onEvent.connect([this] (auto msg) {
            m_events.push_back(std::move(msg));
        });
        m_connection->onMessage.connect([this] (auto msg) {
            m_messages.push_back(std::move(msg));
        });

        m_connection->connect(m_address);
    } else if (argc == 1)
        help();
        // NOTREACHED

    // Build a vector of arguments.
    std::vector<std::string> args;

    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);

    auto commands = exec(args);

    for (const auto &r : commands) {
        auto name = r.find("response");

        if (name == r.end() || !name->is_string())
            log::warning() << "unknown irccd response with no response" << std::endl;

        auto it = m_commandService.find(*name);

        it->result(*this, r);
    }
}

} // !irccd
