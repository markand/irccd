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

#include "command.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "service.hpp"
#include "string_util.hpp"
#include "system.hpp"
#include "config.hpp"
#include "irccd.hpp"

#if defined(HAVE_JS)
#   include <js_directory_module.hpp>
#   include <js_elapsed_timer_module.hpp>
#   include <js_file_module.hpp>
#   include <js_irccd_module.hpp>
#   include <js_logger_module.hpp>
#   include <js_plugin_module.hpp>
#   include <js_server_module.hpp>
#   include <js_system_module.hpp>
#   include <js_timer_module.hpp>
#   include <js_unicode_module.hpp>
#   include <js_util_module.hpp>
#   include "js_plugin.hpp"
#endif

namespace irccd {

namespace {

std::unique_ptr<irccd> instance;

void usage()
{
    std::cerr << "usage: " << sys::program_name() << " [options...]\n\n";
    std::cerr << "Available options:\n";
    std::cerr << "  -c, --config file       specify the configuration file\n";
    std::cerr << "  -f, --foreground        do not run as a daemon\n";
    std::cerr << "  -h, --help              show this help\n";
    std::cerr << "  -v, --verbose           be verbose\n";
    std::cerr << "      --version           show the version\n";
    std::exit(1);
}

void version(const option::result& options)
{
    std::cout << IRCCD_VERSION << std::endl;

    if (options.count("-v") > 0 || options.count("--verbose") > 0) {
        bool ssl = false;
        bool js = false;

#if defined(HAVE_SSL)
        ssl = true;
#endif
#if defined(HAVE_JS)
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

void init(int& argc, char**& argv)
{
    // Needed for some components.
    sys::set_program_name("irccd");

    // Default logging to console.
    log::set_verbose(false);

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

option::result parse(int& argc, char**& argv)
{
    // Parse command line options.
    option::result result;

    try {
        option::options options{
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

        for (const auto& pair : result) {
            if (pair.first == "-h" || pair.first == "--help")
                usage();
                // NOTREACHED
            if (pair.first == "--version")
                version(result);
                // NOTREACHED
            if (pair.first == "-v" || pair.first == "--verbose")
                log::set_verbose(true);
        }
    } catch (const std::exception& ex) {
        log::warning() << sys::program_name() << ": " << ex.what() << std::endl;
        usage();
    }

    return result;
}

config open(const option::result& result)
{
    auto it = result.find("-c");

    if (it != result.end() || (it = result.find("--config")) != result.end()) {
        try {
            return config(it->second);
        } catch (const std::exception &ex) {
            throw std::runtime_error(string_util::sprintf("%s: %s", it->second, ex.what()));
        }
    }

    return config::find();
}

void load_pid(const std::string& path)
{
    if (path.empty())
        return;

    try {
#if defined(HAVE_GETPID)
        std::ofstream out(path, std::ofstream::trunc);

        if (!out)
            throw std::runtime_error(string_util::sprintf("could not open pidfile %s: %s",
                path, std::strerror(errno)));

        log::debug() << "irccd: pid written in " << path << std::endl;
        out << getpid() << std::endl;
#else
        throw std::runtime_error("pidfile option not supported on this platform");
#endif
    } catch (const std::exception& ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void load_gid(const std::string& gid)
{
    try {
        if (!gid.empty())
#if defined(HAVE_SETGID)
            sys::set_gid(gid);
#else
            throw std::runtime_error(" gid option not supported on this platform");
#endif
    } catch (const std::exception& ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void load_uid(const std::string& uid)
{
    try {
        if (!uid.empty())
#if defined(HAVE_SETUID)
            sys::set_uid(uid);
#else
            throw std::runtime_error("uid option not supported on this platform");
#endif
    } catch (const std::exception& ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void load_foreground(bool foreground, const option::result& options)
{
    try {
#if defined(HAVE_DAEMON)
        if (options.count("-f") == 0 && options.count("--foreground") == 0 && !foreground)
            daemon(1, 0);
#else
        if (options.count("-f") > 0 || options.count("--foreground") > 0 || foreground)
            throw std::runtime_error("foreground option not supported on this platform");
#endif
    } catch (const std::exception& ex) {
        log::warning() << "irccd: " << ex.what() << std::endl;
    }
}

void load(const config& config, const option::result& options)
{
    instance->set_config(config.path());

    /*
     * Order matters, please be careful when changing this.
     *
     * 1. Open logs as early as possible to use the defined outputs on any
     *    loading errors.
     */

    // [logs] and [format] sections.
    config.load_logs();
    config.load_formats();

    // Show message here to use the formats.
    log::info() << "irccd: using " << config.path() << std::endl;

    // [general] section.
    load_pid(config.pidfile());
    load_gid(config.gid());
    load_uid(config.uid());
    load_foreground(config.is_foreground(), options);

    // [transport]
    for (const auto& transport : config.load_transports())
        instance->transports().add(transport);

    // [server] section.
    for (const auto& server : config.load_servers())
        instance->servers().add(server);

    // [rule] section.
    for (const auto& rule : config.load_rules())
        instance->rules().add(rule);

    // [plugin] section.
    config.load_plugins(*instance);
}

} // !namespace

} // !irccd

int main(int argc, char** argv)
{
    using namespace irccd;

    init(argc, argv);

    option::result options = parse(argc, argv);

    instance = std::make_unique<class irccd>();
    instance->commands().add(std::make_unique<plugin_config_command>());
    instance->commands().add(std::make_unique<plugin_info_command>());
    instance->commands().add(std::make_unique<plugin_list_command>());
    instance->commands().add(std::make_unique<plugin_load_command>());
    instance->commands().add(std::make_unique<plugin_reload_command>());
    instance->commands().add(std::make_unique<plugin_unload_command>());
    instance->commands().add(std::make_unique<server_channel_mode_command>());
    instance->commands().add(std::make_unique<server_channel_notice_command>());
    instance->commands().add(std::make_unique<server_connect_command>());
    instance->commands().add(std::make_unique<server_disconnect_command>());
    instance->commands().add(std::make_unique<server_info_command>());
    instance->commands().add(std::make_unique<server_invite_command>());
    instance->commands().add(std::make_unique<server_join_command>());
    instance->commands().add(std::make_unique<server_kick_command>());
    instance->commands().add(std::make_unique<server_list_command>());
    instance->commands().add(std::make_unique<server_me_command>());
    instance->commands().add(std::make_unique<server_message_command>());
    instance->commands().add(std::make_unique<server_mode_command>());
    instance->commands().add(std::make_unique<server_nick_command>());
    instance->commands().add(std::make_unique<server_notice_command>());
    instance->commands().add(std::make_unique<server_part_command>());
    instance->commands().add(std::make_unique<server_reconnect_command>());
    instance->commands().add(std::make_unique<server_topic_command>());
    instance->commands().add(std::make_unique<rule_add_command>());
    instance->commands().add(std::make_unique<rule_edit_command>());
    instance->commands().add(std::make_unique<rule_info_command>());
    instance->commands().add(std::make_unique<rule_list_command>());
    instance->commands().add(std::make_unique<rule_move_command>());
    instance->commands().add(std::make_unique<rule_remove_command>());

    // Load Javascript API and plugin loader.
#if defined(HAVE_JS)
    auto loader = std::make_unique<js_plugin_loader>(*instance);

    loader->add_module(std::make_unique<js_irccd_module>());
    loader->add_module(std::make_unique<js_directory_module>());
    loader->add_module(std::make_unique<js_elapsed_timer_module>());
    loader->add_module(std::make_unique<js_file_module>());
    loader->add_module(std::make_unique<js_logger_module>());
    loader->add_module(std::make_unique<js_plugin_module>());
    loader->add_module(std::make_unique<js_server_module>());
    loader->add_module(std::make_unique<js_system_module>());
    loader->add_module(std::make_unique<js_timer_module>());
    loader->add_module(std::make_unique<js_unicode_module>());
    loader->add_module(std::make_unique<js_util_module>());

    instance->plugins().add_loader(std::move(loader));
#endif

    try {
        load(open(options), options);
    } catch (const std::exception& ex) {
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
