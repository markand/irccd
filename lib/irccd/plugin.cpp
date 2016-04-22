/*
 * plugin.cpp -- irccd JavaScript plugin interface
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

#include <stdexcept>

#include "sysconfig.hpp"

#if defined(HAVE_STAT)
#  include <sys/stat.h>
#  include <cerrno>
#  include <cstring>
#endif

#include "fs.hpp"
#include "js-directory.hpp"
#include "js-elapsed-timer.hpp"
#include "js-file.hpp"
#include "js-irccd.hpp"
#include "js-logger.hpp"
#include "js-plugin.hpp"
#include "js-server.hpp"
#include "js-system.hpp"
#include "js-timer.hpp"
#include "js-unicode.hpp"
#include "js-util.hpp"
#include "path.hpp"
#include "plugin.hpp"
#include "server.hpp"
#include "util.hpp"

using namespace std;

namespace irccd {

void Plugin::call(const string &name, unsigned nargs)
{
	duk::getGlobal<void>(m_context, name);

	if (duk::type(m_context, -1) == DUK_TYPE_UNDEFINED) {
		/* Function not defined, remove the undefined value and all arguments */
		duk::pop(m_context, nargs + 1);
	} else {
		/* Call the function and discard the result */
		duk::insert(m_context, -nargs - 1);
		duk::pcall(m_context, nargs);
		duk::pop(m_context);
	}
}

void Plugin::putVars()
{
	duk::StackAssert sa{m_context};

	/* Save a reference to this */
	duk::putGlobal(m_context, "\xff""\xff""plugin", duk::RawPointer<Plugin>{this});
	duk::putGlobal(m_context, "\xff""\xff""name", m_info.name);
	duk::putGlobal(m_context, "\xff""\xff""path", m_info.path);
	duk::putGlobal(m_context, "\xff""\xff""parent", m_info.parent);
}

void Plugin::putPath(const std::string &varname, const std::string &append, path::Path type)
{
	duk::StackAssert sa(m_context);

	bool found = true;
	std::string foundpath;

	/*
	 * Use the first existing directory available.
	 */
	for (const std::string &p : path::list(type)) {
		foundpath = path::clean(p + append);

		if (fs::exists(foundpath)) {
			found = true;
			break;
		}
	}

	/* Use the system as default */
	if (!found)
		foundpath = path::clean(path::get(type, path::OwnerSystem) + append);

	duk::getGlobal<void>(m_context, "Irccd");
	duk::getProperty<void>(m_context, -1, "Plugin");
	duk::putProperty(m_context, -1, varname, foundpath);
	duk::pop(m_context, 2);
}

void Plugin::putPaths()
{
	duk::StackAssert sa(m_context);

	/*
	 * dataPath: DATA + plugin/name (e.g ~/.local/share/irccd/plugins/<name>/)
	 * configPath: CONFIG + plugin/name (e.g ~/.config/irccd/plugin/<name>/)
	 */
	putPath("dataPath", "plugin/" + m_info.name, path::PathData);
	putPath("configPath", "plugin/" + m_info.name, path::PathConfig);
	putPath("cachePath", "plugin/" + m_info.name, path::PathCache);
}

void Plugin::putConfig(const PluginConfig &config)
{
	duk::StackAssert sa(m_context);

	// TODO: override dataPath, configPath, cachePath

	/* Store plugin configuration into Irccd.Plugin.config */
	duk::getGlobal<void>(m_context, "Irccd");
	duk::getProperty<void>(m_context, -1, "Plugin");
	duk::getProperty<void>(m_context, -1, "config");

	if (duk::type(m_context, -1) != DUK_TYPE_OBJECT) {
		duk::pop(m_context);
		duk::push(m_context, duk::Object{});
	}

	for (const auto &pair : config)
		duk::putProperty(m_context, -1, pair.first, pair.second);

	duk::putProperty(m_context, -2, "config");
	duk::pop(m_context, 2);
}

Plugin::Plugin(std::string name, std::string path, const PluginConfig &config)
{
	duk::StackAssert sa(m_context);

	m_info.name = std::move(name);
	m_info.path = std::move(path);

	/*
	 * Duktape currently emit useless warnings when a file do
	 * not exists so we do a homemade access.
	 */
#if defined(HAVE_STAT)
	struct stat st;

	if (stat(m_info.path.c_str(), &st) < 0)
		throw std::runtime_error(std::strerror(errno));
#endif

	/*
	 * Store the base path to the plugin, it is required for
	 * Duktape.modSearch to find external modules and other
	 * sources.
	 *
	 * If path is absolute, the parent is the directory name, otherwise
	 * we use the current working directory (needed for some tests).
	 */
	if (fs::isAbsolute(m_info.path))
		m_info.parent = fs::dirName(m_info.path);
	else
		m_info.parent = fs::cwd();

	/* Load standard irccd API */
	loadJsIrccd(m_context);
	loadJsDirectory(m_context);
	loadJsElapsedTimer(m_context);
	loadJsFile(m_context);
	loadJsLogger(m_context);
	loadJsPlugin(m_context);
	loadJsServer(m_context);
	loadJsSystem(m_context);
	loadJsTimer(m_context);
	loadJsUnicode(m_context);
	loadJsUtil(m_context);

	putVars();
	putPaths();

	/* Try to load the file (does not call onLoad yet) */
	if (duk::pevalFile(m_context, m_info.path) != 0)
		throw duk::error(m_context, -1);

	duk::pop(m_context);

	/* Initialize user defined options after loading to allow the plugin to define default values */
	putConfig(config);

	/* Read metadata */
	duk::getGlobal<void>(m_context, "info");

	if (duk::type(m_context, -1) == DUK_TYPE_OBJECT) {
		m_info.author = duk::optionalProperty<std::string>(m_context, -1, "author", "unknown");
		m_info.license = duk::optionalProperty<std::string>(m_context, -1, "license", "unknown");
		m_info.summary = duk::optionalProperty<std::string>(m_context, -1, "summary", "unknown");
		m_info.version = duk::optionalProperty<std::string>(m_context, -1, "version", "unknown");
	}

	duk::pop(m_context);

	log::debug() << "plugin " << m_info.name << ": " << std::endl;
	log::debug() << "  author:  " << m_info.author << std::endl;
	log::debug() << "  license: " << m_info.license << std::endl;
	log::debug() << "  summary: " << m_info.summary << std::endl;
	log::debug() << "  version: " << m_info.version << std::endl;
}

Plugin::~Plugin()
{
	for (auto &timer : m_timers)
		timer->stop();
}

const PluginInfo &Plugin::info() const
{
	return m_info;
}

void Plugin::addTimer(std::shared_ptr<Timer> timer) noexcept
{
	std::weak_ptr<Timer> ptr(timer);

	/*
	 * These signals are called from the Timer thread and are transmitted to irccd so that it can
	 * calls appropriate timer functions.
	 */
	timer->onSignal.connect([this, ptr] () {
		auto timer = ptr.lock();

		if (timer)
			onTimerSignal(move(timer));
	});
	timer->onEnd.connect([this, ptr] () {
		auto timer = ptr.lock();

		if (timer)
			onTimerEnd(move(timer));
	});

	m_timers.insert(move(timer));
}

void Plugin::removeTimer(const std::shared_ptr<Timer> &timer) noexcept
{
	duk::StackAssert sa(m_context);

	/* Remove the JavaScript function */
	duk::push(m_context, duk::Null{});
	duk::putGlobal(m_context, "\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));

	/* Remove from list */
	m_timers.erase(timer);
}

void Plugin::onChannelMode(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string mode, std::string arg)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(mode));
	duk::push(m_context, move(arg));
	call("onChannelMode", 5);
}

void Plugin::onChannelNotice(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string notice)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(notice));
	call("onChannelNotice", 4);
}

void Plugin::onCommand(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(message));
	call("onCommand", 4);
}

void Plugin::onConnect(std::shared_ptr<Server> server)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	call("onConnect", 1);
}

void Plugin::onInvite(std::shared_ptr<Server> server, std::string origin, std::string channel)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	call("onInvite", 3);
}

void Plugin::onJoin(std::shared_ptr<Server> server, std::string origin, std::string channel)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	call("onJoin", 3);
}

void Plugin::onKick(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string target, std::string reason)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(target));
	duk::push(m_context, move(reason));
	call("onKick", 5);
}

void Plugin::onLoad()
{
	duk::StackAssert sa(m_context);

	call("onLoad", 0);
}

void Plugin::onMessage(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(message));
	call("onMessage", 4);
}

void Plugin::onMe(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(message));
	call("onMe", 4);
}

void Plugin::onMode(std::shared_ptr<Server> server, std::string origin, std::string mode)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(mode));
	call("onMode", 3);
}

void Plugin::onNames(std::shared_ptr<Server> server, std::string channel, std::vector<std::string> names)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(channel));
	duk::push(m_context, move(names));
	call("onNames", 3);
}

void Plugin::onNick(std::shared_ptr<Server> server, std::string oldnick, std::string newnick)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(oldnick));
	duk::push(m_context, move(newnick));
	call("onNick", 3);
}

void Plugin::onNotice(std::shared_ptr<Server> server, std::string origin, std::string notice)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(notice));
	call("onNotice", 3);
}

void Plugin::onPart(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string reason)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(reason));
	call("onPart", 4);
}

void Plugin::onQuery(std::shared_ptr<Server> server, std::string origin, std::string message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(message));
	call("onQuery", 3);
}

void Plugin::onQueryCommand(std::shared_ptr<Server> server, std::string origin, std::string message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(message));
	call("onQueryCommand", 3);
}

void Plugin::onReload()
{
	duk::StackAssert sa(m_context);

	call("onReload");
}

void Plugin::onTopic(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string topic)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, move(origin));
	duk::push(m_context, move(channel));
	duk::push(m_context, move(topic));
	call("onTopic", 4);
}

void Plugin::onUnload()
{
	duk::StackAssert sa(m_context);

	call("onUnload");
}

void Plugin::onWhois(std::shared_ptr<Server> server, ServerWhois whois)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, duk::Object{});
	duk::putProperty(m_context, -1, "nickname", whois.nick);
	duk::putProperty(m_context, -1, "username", whois.user);
	duk::putProperty(m_context, -1, "realname", whois.realname);
	duk::putProperty(m_context, -1, "host", whois.host);
	duk::putProperty(m_context, 1, "channels", whois.channels);
	call("onWhois", 2);
}

namespace duk {

void TypeTraits<irccd::PluginInfo>::push(ContextPtr ctx, const PluginInfo &info)
{
	duk::StackAssert sa(ctx, 1);

	duk::push(ctx, duk::Object{});
	duk::putProperty(ctx, -1, "name", info.name);
	duk::putProperty(ctx, -1, "author", info.author);
	duk::putProperty(ctx, -1, "license", info.license);
	duk::putProperty(ctx, -1, "summary", info.summary);
	duk::putProperty(ctx, -1, "version", info.version);
}

} // !js

} // !irccd
