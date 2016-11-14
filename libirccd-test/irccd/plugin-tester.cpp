/*
 * plugin-tester.cpp -- test fixture helper for javascript plugins
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

#include "mod-directory.hpp"
#include "mod-elapsed-timer.hpp"
#include "mod-file.hpp"
#include "mod-irccd.hpp"
#include "mod-logger.hpp"
#include "mod-plugin.hpp"
#include "mod-server.hpp"
#include "mod-system.hpp"
#include "mod-timer.hpp"
#include "mod-unicode.hpp"
#include "mod-util.hpp"
#include "plugin-js.hpp"
#include "plugin-tester.hpp"
#include "service.hpp"

namespace irccd {

PluginTester::PluginTester()
{
    auto loader = std::make_unique<JsPluginLoader>(m_irccd);

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

    m_irccd.plugins().addLoader(std::move(loader));
}

} // !irccd
