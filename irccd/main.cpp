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

#include <irccd/logger.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include <irccd/daemon/command_service.hpp>
#include <irccd/daemon/config.hpp>
#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/rule_service.hpp>
#include <irccd/daemon/server_service.hpp>
#include <irccd/daemon/transport_service.hpp>

#if defined(HAVE_JS)
#   include <irccd/js/directory_jsapi.hpp>
#   include <irccd/js/elapsed_timer_jsapi.hpp>
#   include <irccd/js/file_jsapi.hpp>
#   include <irccd/js/irccd_jsapi.hpp>
#   include <irccd/js/js_plugin.hpp>
#   include <irccd/js/logger_jsapi.hpp>
#   include <irccd/js/plugin_jsapi.hpp>
#   include <irccd/js/server_jsapi.hpp>
#   include <irccd/js/system_jsapi.hpp>
#   include <irccd/js/timer_jsapi.hpp>
#   include <irccd/js/unicode_jsapi.hpp>
#   include <irccd/js/util_jsapi.hpp>
#endif

namespace irccd {

namespace {

std::atomic<bool> running{true};
std::unique_ptr<irccd> instance;

void usage()
{
    std::cerr << "usage: irccd [options...]\n\n";
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

void init(int& argc, char**& argv)
{
    // Needed for some components.
    sys::set_program_name("irccd");

    // Default logging to console.
    log::set_verbose(false);

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
        log::warning() << "irccd: " << ex.what() << std::endl;
        usage();
    }

    return result;
}

config open(const option::result& result)
{
    auto it = result.find("-c");

    if (it != result.end() || (it = result.find("--config")) != result.end())
        return config(it->second);

    return config::find("irccd.conf");
}

} // !namespace

} // !irccd

int main(int argc, char** argv)
{
    using namespace irccd;

    init(argc, argv);

    boost::asio::io_service service;
    boost::asio::signal_set sigs(service, SIGINT, SIGTERM);

    auto options = parse(argc, argv);

    instance = std::make_unique<class irccd>(service);
    instance->commands().add(std::make_unique<plugin_config_command>());
    instance->commands().add(std::make_unique<plugin_info_command>());
    instance->commands().add(std::make_unique<plugin_list_command>());
    instance->commands().add(std::make_unique<plugin_load_command>());
    instance->commands().add(std::make_unique<plugin_reload_command>());
    instance->commands().add(std::make_unique<plugin_unload_command>());
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

    loader->modules().push_back(std::make_unique<irccd_jsapi>());
    loader->modules().push_back(std::make_unique<directory_jsapi>());
    loader->modules().push_back(std::make_unique<elapsed_timer_jsapi>());
    loader->modules().push_back(std::make_unique<file_jsapi>());
    loader->modules().push_back(std::make_unique<logger_jsapi>());
    loader->modules().push_back(std::make_unique<plugin_jsapi>());
    loader->modules().push_back(std::make_unique<server_jsapi>());
    loader->modules().push_back(std::make_unique<system_jsapi>());
    loader->modules().push_back(std::make_unique<timer_jsapi>());
    loader->modules().push_back(std::make_unique<unicode_jsapi>());
    loader->modules().push_back(std::make_unique<util_jsapi>());

    instance->plugins().add_loader(std::move(loader));
#endif

    try {
        instance->set_config(open(options));
        instance->load();
    } catch (const std::exception& ex) {
        log::warning() << "abort: " << ex.what() << std::endl;
        return 1;
    }

    /*
     * Assign instance to nullptr to force deletion of irccd and all its
     * associated objects before any other static global values such as
     * loggers.
     */
    sigs.async_wait([&] (auto, auto) {
        running = false;
        service.stop();
    });

    while (running)
        service.run();

    instance = nullptr;
}
