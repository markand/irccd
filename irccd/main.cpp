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

#include <irccd/sysconfig.hpp>

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

#include <irccd/logger.hpp>
#include <irccd/options.hpp>
#include <irccd/path.hpp>
#include <irccd/service-plugin.hpp>
#include <irccd/service-rule.hpp>
#include <irccd/service-server.hpp>
#include <irccd/service-transport.hpp>
#include <irccd/system.hpp>
#include <irccd/config.hpp>
#include <irccd/irccd.hpp>

using namespace fmt::literals;

using namespace irccd;

namespace {

std::unique_ptr<Irccd> instance;

void usage()
{
    log::warning() << "usage: " << sys::programName() << " [options...]\n\n";
    log::warning() << "Available options:\n";
    log::warning() << "  -c, --config file       specify the configuration file\n";
    log::warning() << "  -f, --foreground        do not run as a daemon\n";
    log::warning() << "      --help              show this help\n";
    log::warning() << "  -p, --plugin name       load a specific plugin\n";
    log::warning() << "  -v, --verbose           be verbose\n";
    log::warning() << "      --version           show the version\n";
    std::exit(1);
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
    log::setInterface(std::make_unique<log::Console>());

    // Register some signals.
    signal(SIGINT, stop);
    signal(SIGTERM, stop);

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
            { "--help",         false   },
            { "-p",             true    },
            { "--plugin",       true    },
            { "-v",             false   },
            { "--verbose",      false   },
            { "--version",      false   }
        };

        result = option::read(argc, argv, options);

        for (const auto &pair : result) {
            if (pair.first == "--help")
                usage();
                // NOTREACHED
            if (pair.first == "--version") {
                std::cout << IRCCD_VERSION << std::endl;
                std::exit(1);
            }
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
     * 1. Open logs as early as possible to use the defined outputs on any loading errors.
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
        instance->transportService().add(transport);

    // [server] section.
    for (const auto &server : config.loadServers())
        instance->serverService().add(server);

    // [rule] section.
    for (const auto &rule : config.loadRules())
        instance->ruleService().add(rule);

    // [plugin] section.
    config.loadPlugins(*instance);
}

} // !namespace

int main(int argc, char **argv)
{
    init(argc, argv);

    option::Result options = parse(argc, argv);

    // Find configuration file.
    instance = std::make_unique<Irccd>();

    try {
        load(open(options), options);
    } catch (const std::exception &ex) {
        log::warning() << "error: " << ex.what() << std::endl;
        return 1;
    }

    instance->run();

    return 0;
}
