/*
 * service-module.cpp -- store and manage JavaScript modules
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

#include <algorithm>

#include "mod-elapsed-timer.hpp"
#include "mod-directory.hpp"
#include "mod-file.hpp"
#include "mod-irccd.hpp"
#include "mod-logger.hpp"
#include "mod-plugin.hpp"
#include "mod-server.hpp"
#include "mod-system.hpp"
#include "mod-timer.hpp"
#include "mod-unicode.hpp"
#include "mod-util.hpp"
#include "service-module.hpp"

namespace irccd {

namespace {

auto find(const std::vector<std::shared_ptr<Module>> &modules, const std::string &name) noexcept
{
	return std::find_if(modules.begin(), modules.end(), [&] (const auto &module) {
		return module->name() == name;
	});
}

} // !namespace

ModuleService::ModuleService()
{
	// Load Irccd global first.
	m_modules.push_back(std::make_shared<IrccdModule>());

	// Additional modules.
	m_modules.push_back(std::make_shared<ElapsedTimerModule>());
	m_modules.push_back(std::make_shared<DirectoryModule>());
	m_modules.push_back(std::make_shared<FileModule>());
	m_modules.push_back(std::make_shared<LoggerModule>());
	m_modules.push_back(std::make_shared<PluginModule>());
	m_modules.push_back(std::make_shared<ServerModule>());
	m_modules.push_back(std::make_shared<SystemModule>());
	m_modules.push_back(std::make_shared<TimerModule>());
	m_modules.push_back(std::make_shared<UnicodeModule>());
	m_modules.push_back(std::make_shared<UtilModule>());
}

bool ModuleService::contains(const std::string &name) const
{
	return find(m_modules, name) != m_modules.end();
}

void ModuleService::add(std::shared_ptr<Module> module)
{
	assert(!contains(module->name()));

	m_modules.push_back(std::move(module));
}

} // !irccd
