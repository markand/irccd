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

#include <csignal>
#include <iostream>

#include <irccd/config.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/dynlib_plugin.hpp>
#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>

#include <irccd/daemon/service/plugin_service.hpp>
#include <irccd/daemon/service/rule_service.hpp>
#include <irccd/daemon/service/server_service.hpp>
#include <irccd/daemon/service/transport_service.hpp>

#if defined(IRCCD_HAVE_JS)
#   include <irccd/js/js_plugin.hpp>
#endif

namespace irccd {

namespace {

std::unique_ptr<irccd> instance;

// {{{ usage

void usage()
{
    std::cerr << "usage: irccd [options...]\n\n";
    std::cerr << "Available options:\n";
    std::cerr << "  -c, --config file       specify the configuration file\n";
    std::cerr << "  -h, --help              show this help\n";
    std::cerr << "  -v, --verbose           be verbose\n";
    std::cerr << "      --version           show the version\n";
    std::exit(1);
}

// }}}

// {{{ version

void version(const option::result& options)
{
    std::cout << IRCCD_VERSION << std::endl;

    if (options.count("-v") > 0 || options.count("--verbose") > 0) {
        bool ssl = false;
        bool js = false;

#if defined(IRCCD_HAVE_SSL)
        ssl = true;
#endif
#if defined(IRCCD_HAVE_JS)
        js = true;
#endif

        std::cout << std::boolalpha << std::endl;
        std::cout << "ssl:          " << ssl << std::endl;
        std::cout << "javascript:   " << js << std::endl;
    }

    std::exit(0);
}

// }}}

// {{{ init

void init(int& argc, char**& argv)
{
    // Needed for some components.
    sys::set_program_name("irccd");

    // Default logging to console.
    instance->get_log().set_verbose(false);

    -- argc;
    ++ argv;
}

// }}}

// {{{ parse

option::result parse(int& argc, char**& argv)
{
    // Parse command line options.
    option::result result;

    try {
        option::options options{
            { "-c",             true    },
            { "--config",       true    },
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
                instance->get_log().set_verbose(true);
        }
    } catch (const std::exception& ex) {
        instance->get_log().warning("irccd", "") << "abort: " << ex.what() << std::endl;
        usage();
    }

    return result;
}

// }}}

// {{{ open

config open(const option::result& result)
{
    auto it = result.find("-c");

    if (it != result.end() || (it = result.find("--config")) != result.end())
        return config(it->second);

    auto cfg = config::search("irccd.conf");

    if (!cfg)
        throw std::runtime_error("no configuration file could be found");

    return *cfg;
}

// }}}

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

#if defined(IRCCD_HAVE_JS)
    instance->plugins().add_loader(js_plugin_loader::defaults(*instance));
#endif

    instance->plugins().add_loader(std::make_unique<dynlib_plugin_loader>());

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
        service.stop();
    });

    try {
        service.run();
    } catch (const std::exception& ex) {
        std::cerr << "abort: " << ex.what() << std::endl;
        return 1;
    }

    instance = nullptr;
}
