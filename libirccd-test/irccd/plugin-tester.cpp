/*
 * plugin-tester.cpp -- test fixture helper for javascript plugins
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

#include "js_directory_module.hpp"
#include "js_elapsed_timer_module.hpp"
#include "js_file_module.hpp"
#include "js_irccd_module.hpp"
#include "js_logger_module.hpp"
#include "js_plugin_module.hpp"
#include "js_server_module.hpp"
#include "js_system_module.hpp"
#include "js_timer_module.hpp"
#include "js_unicode_module.hpp"
#include "js_util_module.hpp"
#include "js_plugin.hpp"
#include "plugin-tester.hpp"
#include "service.hpp"

namespace irccd {

PluginTester::PluginTester()
{
    auto loader = std::make_unique<js_plugin_loader>(m_irccd);

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

    m_irccd.plugins().add_loader(std::move(loader));
}

} // !irccd
