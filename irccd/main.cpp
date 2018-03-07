/*
 * main.cpp -- irccd main file
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

#include <irccd/config.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/plugin_config_command.hpp>
#include <irccd/daemon/plugin_info_command.hpp>
#include <irccd/daemon/plugin_list_command.hpp>
#include <irccd/daemon/plugin_list_command.hpp>
#include <irccd/daemon/plugin_load_command.hpp>
#include <irccd/daemon/plugin_reload_command.hpp>
#include <irccd/daemon/plugin_unload_command.hpp>
#include <irccd/daemon/rule_add_command.hpp>
#include <irccd/daemon/rule_edit_command.hpp>
#include <irccd/daemon/rule_info_command.hpp>
#include <irccd/daemon/rule_list_command.hpp>
#include <irccd/daemon/rule_move_command.hpp>
#include <irccd/daemon/rule_remove_command.hpp>
#include <irccd/daemon/server_connect_command.hpp>
#include <irccd/daemon/server_disconnect_command.hpp>
#include <irccd/daemon/server_info_command.hpp>
#include <irccd/daemon/server_invite_command.hpp>
#include <irccd/daemon/server_join_command.hpp>
#include <irccd/daemon/server_kick_command.hpp>
#include <irccd/daemon/server_list_command.hpp>
#include <irccd/daemon/server_me_command.hpp>
#include <irccd/daemon/server_message_command.hpp>
#include <irccd/daemon/server_mode_command.hpp>
#include <irccd/daemon/server_nick_command.hpp>
#include <irccd/daemon/server_notice_command.hpp>
#include <irccd/daemon/server_part_command.hpp>
#include <irccd/daemon/server_reconnect_command.hpp>
#include <irccd/daemon/server_topic_command.hpp>

#include <irccd/daemon/service/plugin_service.hpp>
#include <irccd/daemon/service/rule_service.hpp>
#include <irccd/daemon/service/server_service.hpp>
#include <irccd/daemon/service/transport_service.hpp>

#if defined(HAVE_JS)
#   include <irccd/js/js_plugin.hpp>
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
    instance->log().set_verbose(false);

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
                instance->log().set_verbose(true);
        }
    } catch (const std::exception& ex) {
        instance->log().warning() << "irccd: " << ex.what() << std::endl;
        usage();
    }

    return result;
}

config open(const option::result& result)
{
    auto it = result.find("-c");

    if (it != result.end() || (it = result.find("--config")) != result.end())
        return config(it->second);

    auto cfg = config::find("irccd.conf");

    if (!cfg)
        throw std::runtime_error("no configuration file could be found");

    return *cfg;
}

} // !namespace

} // !irccd

int main(int argc, char** argv)
{
    using namespace irccd;

    boost::asio::io_service service;
    boost::asio::signal_set sigs(service, SIGINT, SIGTERM);

    instance = std::make_unique<class irccd>(service);

    init(argc, argv);

    auto options = parse(argc, argv);

    instance->transports().get_commands().push_back(std::make_unique<plugin_config_command>());
    instance->transports().get_commands().push_back(std::make_unique<plugin_info_command>());
    instance->transports().get_commands().push_back(std::make_unique<plugin_list_command>());
    instance->transports().get_commands().push_back(std::make_unique<plugin_load_command>());
    instance->transports().get_commands().push_back(std::make_unique<plugin_reload_command>());
    instance->transports().get_commands().push_back(std::make_unique<plugin_unload_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_connect_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_disconnect_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_info_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_invite_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_join_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_kick_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_list_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_me_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_message_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_mode_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_nick_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_notice_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_part_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_reconnect_command>());
    instance->transports().get_commands().push_back(std::make_unique<server_topic_command>());
    instance->transports().get_commands().push_back(std::make_unique<rule_add_command>());
    instance->transports().get_commands().push_back(std::make_unique<rule_edit_command>());
    instance->transports().get_commands().push_back(std::make_unique<rule_info_command>());
    instance->transports().get_commands().push_back(std::make_unique<rule_list_command>());
    instance->transports().get_commands().push_back(std::make_unique<rule_move_command>());
    instance->transports().get_commands().push_back(std::make_unique<rule_remove_command>());

#if defined(HAVE_JS)
    instance->plugins().add_loader(js_plugin_loader::defaults(*instance));
#endif

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
        running = false;
        service.stop();
    });

    while (running)
        service.run();

    instance = nullptr;
}
