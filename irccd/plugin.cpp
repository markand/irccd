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

#include <irccd-config.h>

#if defined(HAVE_STAT)
#  include <sys/stat.h>
#  include <cerrno>
#  include <cstring>
#endif

#include <filesystem.h>
#include <path.h>
#include <util.h>

#include "js-directory.h"
#include "js-elapsed-timer.h"
#include "js-file.h"
#include "js-irccd.h"
#include "js-logger.h"
#include "js-plugin.h"
#include "js-server.h"
#include "js-system.h"
#include "js-timer.h"
#include "js-unicode.h"
#include "js-util.h"

#include "plugin.h"
#include "server.h"

using namespace std;

namespace irccd {

void Plugin::call(const string &name, unsigned nargs)
{
	m_context.getGlobal<void>(name);

	if (m_context.type(-1) == DUK_TYPE_UNDEFINED) {
		/* Function not defined, remove the undefined value and all arguments */
		m_context.pop(nargs + 1);
	} else {
		/* Call the function and discard the result */
		m_context.insert(-nargs - 1);
		m_context.pcall(nargs);
		m_context.pop();
	}
}

void Plugin::putVars()
{
	js::StackAssert sa{m_context};

	/* Save a reference to this */
	m_context.putGlobal("\xff""\xff""plugin", js::RawPointer<Plugin>{this});
	m_context.putGlobal("\xff""\xff""name", m_info.name);
	m_context.putGlobal("\xff""\xff""path", m_info.path);
	m_context.putGlobal("\xff""\xff""parent", m_info.parent);
}

void Plugin::putPath(const std::string &varname, const std::string &append, path::Path type)
{
	js::StackAssert sa(m_context);

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

	m_context.getGlobal<void>("Irccd");
	m_context.getProperty<void>(-1, "Plugin");
	m_context.putProperty(-1, varname, foundpath);
	m_context.pop(2);
}

void Plugin::putPaths()
{
	js::StackAssert sa(m_context);

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
	js::StackAssert sa(m_context);

	// TODO: override dataPath, configPath, cachePath

	/* Store plugin configuration into Irccd.Plugin.config */
	m_context.getGlobal<void>("Irccd");
	m_context.getProperty<void>(-1, "Plugin");
	m_context.getProperty<void>(-1, "config");

	if (m_context.type(-1) != DUK_TYPE_OBJECT) {
		m_context.pop();
		m_context.push(js::Object{});
	}

	m_context.push(config);
	m_context.putProperty(-2, "config");
	m_context.pop(2);
}

Plugin::Plugin(std::string name, std::string path, const PluginConfig &config)
{
	js::StackAssert sa(m_context);

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
	m_context.peval(js::File{m_info.path});
	m_context.pop();

	/* Initialize user defined options after loading to allow the plugin to define default values */
	putConfig(config);

	/* Read metadata */
	m_context.getGlobal<void>("info");

	if (m_context.type(-1) == DUK_TYPE_OBJECT) {
		m_info.author = m_context.optionalProperty<std::string>(-1, "author", "unknown");
		m_info.license = m_context.optionalProperty<std::string>(-1, "license", "unknown");
		m_info.summary = m_context.optionalProperty<std::string>(-1, "summary", "unknown");
		m_info.version = m_context.optionalProperty<std::string>(-1, "version", "unknown");
	}

	m_context.pop();

	log::debug() << "plugin " << m_info.name << ": " << std::endl;
	log::debug() << "  author:  " << m_info.author << std::endl;
	log::debug() << "  license: " << m_info.license << std::endl;
	log::debug() << "  summary: " << m_info.summary << std::endl;
	log::debug() << "  version: " << m_info.version << std::endl;
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
	/* Remove the JavaScript function */
	m_context.push(js::Null{});
	m_context.putGlobal("\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));

	/* Remove from list */
	m_timers.erase(timer);
}

void Plugin::onChannelMode(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string mode, std::string arg)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(mode));
	m_context.push(move(arg));
	call("onChannelMode", 5);
}

void Plugin::onChannelNotice(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string notice)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(notice));
	call("onChannelNotice", 4);
}

void Plugin::onCommand(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(message));
	call("onCommand", 4);
}

void Plugin::onConnect(std::shared_ptr<Server> server)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	call("onConnect", 1);
}

void Plugin::onInvite(std::shared_ptr<Server> server, std::string origin, std::string channel)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	call("onInvite", 3);
}

void Plugin::onJoin(std::shared_ptr<Server> server, std::string origin, std::string channel)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	call("onJoin", 3);
}

void Plugin::onKick(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string target, std::string reason)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(target));
	m_context.push(move(reason));
	call("onKick", 5);
}

void Plugin::onLoad()
{
	js::StackAssert sa(m_context);

	call("onLoad", 0);
}

void Plugin::onMessage(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(message));
	call("onMessage", 4);
}

void Plugin::onMe(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(message));
	call("onMe", 4);
}

void Plugin::onMode(std::shared_ptr<Server> server, std::string origin, std::string mode)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(mode));
	call("onMode", 3);
}

void Plugin::onNames(std::shared_ptr<Server> server, std::string channel, std::vector<std::string> names)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(channel));
	m_context.push(move(names));
	call("onNames", 3);
}

void Plugin::onNick(std::shared_ptr<Server> server, std::string oldnick, std::string newnick)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(oldnick));
	m_context.push(move(newnick));
	call("onNick", 3);
}

void Plugin::onNotice(std::shared_ptr<Server> server, std::string origin, std::string notice)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(notice));
	call("onNotice", 3);
}

void Plugin::onPart(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string reason)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(reason));
	call("onPart", 4);
}

void Plugin::onQuery(std::shared_ptr<Server> server, std::string origin, std::string message)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(message));
	call("onQuery", 3);
}

void Plugin::onQueryCommand(std::shared_ptr<Server> server, std::string origin, std::string message)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(message));
	call("onQueryCommand", 3);
}

void Plugin::onReload()
{
	js::StackAssert sa(m_context);

	call("onReload");
}

void Plugin::onTopic(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string topic)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(move(origin));
	m_context.push(move(channel));
	m_context.push(move(topic));
	call("onTopic", 4);
}

void Plugin::onUnload()
{
	js::StackAssert sa(m_context);

	call("onUnload");
}

void Plugin::onWhois(std::shared_ptr<Server> server, ServerWhois whois)
{
	js::StackAssert sa(m_context);

	m_context.push(js::Shared<Server>{server});
	m_context.push(js::Object{});
	m_context.putProperty(-1, "nickname", whois.nick);
	m_context.putProperty(-1, "username", whois.user);
	m_context.putProperty(-1, "realname", whois.realname);
	m_context.putProperty(-1, "host", whois.host);
	m_context.putProperty(1, "channels", whois.channels);
	call("onWhois", 2);
}

namespace js {

void TypeInfo<PluginInfo>::push(Context &ctx, const PluginInfo &info)
{
	js::StackAssert sa(ctx, 1);

	ctx.push(js::Object{});
	ctx.putProperty(-1, "name", info.name);
	ctx.putProperty(-1, "author", info.author);
	ctx.putProperty(-1, "license", info.license);
	ctx.putProperty(-1, "summary", info.summary);
	ctx.putProperty(-1, "version", info.version);
}

} // !js

} // !irccd
