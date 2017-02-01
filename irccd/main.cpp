/*
 * main.cpp -- irccd main file
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

#include "sysconfig.hpp"

#if defined(HAVE_GETPID)
#  include <sys/types.h>
#  include <unistd.h>
#  include <cerrno>
#  include <cstring>
#  include <fstream>
#endif

#if defined(HAVE_DAEMON)
#  include <cstdlib>
#endif

#include <csignal>
#include <iostream>

#include <format.h>

#include "command.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "path.hpp"
#include "service.hpp"
#include "system.hpp"
#include "config.hpp"
#include "irccd.hpp"

#if defined(WITH_JS)
#   include "mod-directory.hpp"
#   include "mod-elapsed-timer.hpp"
#   include "mod-file.hpp"
#   include "mod-irccd.hpp"
#   include "mod-logger.hpp"
#   include "mod-plugin.hpp"
#   include "mod-server.hpp"
#   include "mod-system.hpp"
#   include "mod-timer.hpp"
#   include "mod-unicode.hpp"
#   include "mod-util.hpp"
#   include "plugin-js.hpp"
#endif

using namespace fmt::literals;

using namespace irccd;

namespace {

std::unique_ptr<Irccd> instance;

void usage()
{
    std::cerr << "usage: " << sys::programName() << " [options...]\n\n";
    std::cerr << "Available options:\n";
    std::cerr << "  -c, --config file       specify the configuration file\n";
    std::cerr << "  -f, --foreground        do not run as a daemon\n";
    std::cerr << "  -h, --help              show this help\n";
    std::cerr << "  -v, --verbose           be verbose\n";
    std::cerr << "      --version           show the version\n";
    std::exit(1);
}

void version(const option::Result &options)
{
    std::cout << IRCCD_VERSION << std::endl;

    if (options.count("-v") > 0 || options.count("--verbose") > 0) {
        bool ssl = false;
        bool js = false;

#if defined(WITH_SSL)
        ssl = true;
#endif
#if defined(WITH_JS)
        js = true;
#endif

        std::cout << std::boolalpha << std::endl;
        std::cout << "ssl:          " << ssl << std::endl;
        std::cout << "javascript:   " << js << std::endl;
    }

    std::exit(0);
}

void stop(int)
{
    instance->stop();
}

void init(int &argc, char **&argv)
{
    // Needed for some components.
    sys::setProgramName("irccd");
    path::setApplicationPath(argv[0]);

    // Default logging to console.
    log::setVerbose(false);

    // Register some signals.
    signal(SIGINT, stop);
    signal(SIGTERM, stop);

#if defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif

#if defined(SIGQUIT)
    signal(SIGQUIT, stop);
#endif

    -- argc;
    ++ argv;
}

option::Result parse(int &argc, char **&argv)
{
    // Parse command line options.
    option::Result result;

    try {
        option::Options options{
            { "-c",             true    },
            { "--config",       true    },
            { "-f",             false   },
            { "--foreground",   false   },
            { "-h",             false   },
            { "--help",         false   },
            { "-v",             false   },
            { "--verbose",      false   },
            { "--version",      false   }
        };

        result = option::read(argc, argv, options);

        for (const auto &pair : result) {
            if (pair.first == "-h" || pair.first == "--help")
                usage();
                // NOTREACHED
            if (pair.first == "--version")
                version(result);
                // NOTREACHED
            if (pair.first == "-v" || pair.first == "--verbose")
                log::setVerbose(true);
        }
    } catch (const std::exception &ex) {
        log::warning() << sys::programName() << ": " << ex.what() << std::endl;
        usage();
    }

    return result;
}

Config open(const option::Result &result)
{
    auto it = result.find("-c");

    if (it != result.end() || (it = result.find("--config")) != result.end()) {
        try {
            return Config(it->second);
        } catch (const std::exception &ex) {
            throw std::runtime_error("{}: {}"_format(it->second, ex.what()));
        }
    }

    return Config::find();
}

void loadPid(const std::string &path)
{
    if (path.empty())
        return;

    try {
#if defined(HAVE_GETPID)
        std::ofstream out(path, std::ofstream::trunc);

        if (!out)
            throw std::runtime_error("could not open pidfile {}: {}"_format(path, std::strerror(errno)));

        log::debug() << "irccd: pid written in " << path << std::endl;
        out << getpid() << std::endl;
#else
        throw std::runtime_error("pidfile option not supported on this platform");
#endif
    } catch (const std::exception &ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void loadGid(const std::string gid)
{
    try {
        if (!gid.empty())
#if defined(HAVE_SETGID)
            sys::setGid(gid);
#else
            throw std::runtime_error(" gid option not supported on this platform");
#endif
    } catch (const std::exception &ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void loadUid(const std::string &uid)
{
    try {
        if (!uid.empty())
#if defined(HAVE_SETUID)
            sys::setUid(uid);
#else
            throw std::runtime_error("uid option not supported on this platform");
#endif
    } catch (const std::exception &ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void loadForeground(bool foreground, const option::Result &options)
{
    try {
#if defined(HAVE_DAEMON)
        if (options.count("-f") == 0 && options.count("--foreground") == 0 && !foreground)
            daemon(1, 0);
#else
        if (options.count("-f") > 0 || options.count("--foreground") > 0 || foreground)
            throw std::runtime_error("foreground option not supported on this platform");
#endif
    } catch (const std::exception &ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void load(const Config &config, const option::Result &options)
{
    /*
     * Order matters, please be careful when changing this.
     *
     * 1. Open logs as early as possible to use the defined outputs on any
     *    loading errors.
     */

    // [logs] and [format] sections.
    config.loadLogs();
    config.loadFormats();

    // Show message here to use the formats.
    log::info() << "irccd: using " << config.path() << std::endl;

    // [general] section.
    loadPid(config.pidfile());
    loadGid(config.gid());
    loadUid(config.uid());
    loadForeground(config.isForeground(), options);

    // [transport]
    for (const auto &transport : config.loadTransports())
        instance->transports().add(transport);

    // [server] section.
    for (const auto &server : config.loadServers())
        instance->servers().add(server);

    // [rule] section.
    for (const auto &rule : config.loadRules())
        instance->rules().add(rule);

    // [plugin] section.
    config.loadPlugins(*instance);
}

} // !namespace

int main(int argc, char **argv)
{
    init(argc, argv);

    option::Result options = parse(argc, argv);

    instance = std::make_unique<Irccd>();
    instance->commands().add(std::make_unique<command::PluginConfigCommand>());
    instance->commands().add(std::make_unique<command::PluginInfoCommand>());
    instance->commands().add(std::make_unique<command::PluginListCommand>());
    instance->commands().add(std::make_unique<command::PluginLoadCommand>());
    instance->commands().add(std::make_unique<command::PluginReloadCommand>());
    instance->commands().add(std::make_unique<command::PluginUnloadCommand>());
    instance->commands().add(std::make_unique<command::ServerChannelModeCommand>());
    instance->commands().add(std::make_unique<command::ServerChannelNoticeCommand>());
    instance->commands().add(std::make_unique<command::ServerConnectCommand>());
    instance->commands().add(std::make_unique<command::ServerDisconnectCommand>());
    instance->commands().add(std::make_unique<command::ServerInfoCommand>());
    instance->commands().add(std::make_unique<command::ServerInviteCommand>());
    instance->commands().add(std::make_unique<command::ServerJoinCommand>());
    instance->commands().add(std::make_unique<command::ServerKickCommand>());
    instance->commands().add(std::make_unique<command::ServerListCommand>());
    instance->commands().add(std::make_unique<command::ServerMeCommand>());
    instance->commands().add(std::make_unique<command::ServerMessageCommand>());
    instance->commands().add(std::make_unique<command::ServerModeCommand>());
    instance->commands().add(std::make_unique<command::ServerNickCommand>());
    instance->commands().add(std::make_unique<command::ServerNoticeCommand>());
    instance->commands().add(std::make_unique<command::ServerPartCommand>());
    instance->commands().add(std::make_unique<command::ServerReconnectCommand>());
    instance->commands().add(std::make_unique<command::ServerTopicCommand>());

    // Load Javascript API and plugin loader.
#if defined(WITH_JS)
    auto loader = std::make_unique<JsPluginLoader>(*instance);

    loader->addModule(std::make_unique<IrccdModule>());
    loader->addModule(std::make_unique<DirectoryModule>());
    loader->addModule(std::make_unique<ElapsedTimerModule>());
    loader->addModule(std::make_unique<FileModule>());
    loader->addModule(std::make_unique<LoggerModule>());
    loader->addModule(std::make_unique<PluginModule>());
    loader->addModule(std::make_unique<ServerModule>());
    loader->addModule(std::make_unique<SystemModule>());
    loader->addModule(std::make_unique<TimerModule>());
    loader->addModule(std::make_unique<UnicodeModule>());
    loader->addModule(std::make_unique<UtilModule>());

    instance->plugins().addLoader(std::move(loader));
#endif

    try {
        load(open(options), options);
    } catch (const std::exception &ex) {
        log::warning() << "error: " << ex.what() << std::endl;
        return 1;
    }

    /*
     * Assign instance to nullptr to force deletion of irccd and all its
     * associated objects before any other static global values such as
     * loggers.
     */
    instance->run();
    instance = nullptr;
}
