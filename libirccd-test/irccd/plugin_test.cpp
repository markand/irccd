/*
 * plugin_test.cpp -- test fixture helper for Javascript plugins
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

#include <irccd/js_directory_module.hpp>
#include <irccd/js_elapsed_timer_module.hpp>
#include <irccd/js_file_module.hpp>
#include <irccd/js_irccd_module.hpp>
#include <irccd/js_logger_module.hpp>
#include <irccd/js_plugin_module.hpp>
#include <irccd/js_server_module.hpp>
#include <irccd/js_system_module.hpp>
#include <irccd/js_timer_module.hpp>
#include <irccd/js_unicode_module.hpp>
#include <irccd/js_util_module.hpp>
#include <irccd/js_plugin.hpp>
#include <irccd/logger.hpp>
#include <irccd/service.hpp>

#include "plugin_test.hpp"

namespace irccd {

plugin_test::plugin_test(std::string name, std::string path)
    : server_(std::make_shared<journal_server>("test"))
{
    log::set_verbose(false);
    log::set_logger(std::make_unique<log::silent_logger>());

    js_plugin_loader loader(irccd_);

    loader.add_module(std::make_unique<js_irccd_module>());
    loader.add_module(std::make_unique<js_directory_module>());
    loader.add_module(std::make_unique<js_elapsed_timer_module>());
    loader.add_module(std::make_unique<js_file_module>());
    loader.add_module(std::make_unique<js_logger_module>());
    loader.add_module(std::make_unique<js_plugin_module>());
    loader.add_module(std::make_unique<js_server_module>());
    loader.add_module(std::make_unique<js_system_module>());
    loader.add_module(std::make_unique<js_timer_module>());
    loader.add_module(std::make_unique<js_unicode_module>());
    loader.add_module(std::make_unique<js_util_module>());

    plugin_ = loader.open(name, path);
    irccd_.plugins().add(plugin_);
    irccd_.servers().add(server_);
}

} // !irccd