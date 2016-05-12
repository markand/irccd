/*
 * plugin-js.cpp -- JavaScript plugins for irccd
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
#include "logger.hpp"
#include "plugin-js.hpp"

namespace irccd {

void JsPlugin::call(const std::string &name, unsigned nargs)
{
	duk::getGlobal<void>(m_context, name);

	if (duk::type(m_context, -1) == DUK_TYPE_UNDEFINED) {
		/* Function not defined, remove the undefined value and all arguments */
		duk::pop(m_context, nargs + 1);
	} else {
		/* Call the function and discard the result */
		duk::insert(m_context, -nargs - 1);

		if (duk::pcall(m_context, nargs) != 0) {
			auto error = duk::error(m_context, -1);

			duk::pop(m_context);

			throw error;
		} else {
			duk::pop(m_context);
		}
	}
}

void JsPlugin::putVars()
{
	duk::StackAssert sa(m_context);

	/* Save a reference to this */
	duk::putGlobal(m_context, "\xff""\xff""plugin", duk::RawPointer<JsPlugin>{this});
	duk::putGlobal(m_context, "\xff""\xff""name", name());
	duk::putGlobal(m_context, "\xff""\xff""path", path());
}

void JsPlugin::putPath(const std::string &varname, const std::string &append, path::Path type)
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
	if (!found) {
		foundpath = path::clean(path::get(type, path::OwnerSystem) + append);
	}

	duk::getGlobal<void>(m_context, "Irccd");
	duk::getProperty<void>(m_context, -1, "Plugin");
	duk::putProperty(m_context, -1, varname, foundpath);
	duk::pop(m_context, 2);
}

void JsPlugin::putPaths()
{
	duk::StackAssert sa(m_context);

	/*
	 * dataPath: DATA + plugin/name (e.g ~/.local/share/irccd/plugins/<name>/)
	 * configPath: CONFIG + plugin/name (e.g ~/.config/irccd/plugin/<name>/)
	 */
	putPath("dataPath", "plugin/" + name(), path::PathData);
	putPath("configPath", "plugin/" + name(), path::PathConfig);
	putPath("cachePath", "plugin/" + name(), path::PathCache);
}

void JsPlugin::putConfig(const PluginConfig &config)
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

	for (const auto &pair : config) {
		duk::putProperty(m_context, -1, pair.first, pair.second);
	}

	duk::putProperty(m_context, -2, "config");
	duk::pop(m_context, 2);
}

JsPlugin::JsPlugin(std::string name, std::string path, const PluginConfig &config)
	: Plugin(name, path, config)
{
	duk::StackAssert sa(m_context);

	/*
	 * Duktape currently emit useless warnings when a file do
	 * not exists so we do a homemade access.
	 */
#if defined(HAVE_STAT)
	struct stat st;

	if (::stat(path.c_str(), &st) < 0) {
		throw std::runtime_error(std::strerror(errno));
	}
#endif

	// TODO: change with future modules
	// Load standard irccd API.
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
	if (duk::pevalFile(m_context, path) != 0) {
		throw duk::error(m_context, -1);
	}

	duk::pop(m_context);

	/* Initialize user defined options after loading to allow the plugin to define default values */
	putConfig(config);

	/* Read metadata */
	duk::getGlobal<void>(m_context, "info");

	if (duk::type(m_context, -1) == DUK_TYPE_OBJECT) {
		setAuthor(duk::optionalProperty<std::string>(m_context, -1, "author", author()));
		setLicense(duk::optionalProperty<std::string>(m_context, -1, "license", license()));
		setSummary(duk::optionalProperty<std::string>(m_context, -1, "summary", summary()));
		setVersion(duk::optionalProperty<std::string>(m_context, -1, "version", version()));
	}

	duk::pop(m_context);

	log::debug() << "plugin " << name << ": " << std::endl;
	log::debug() << "  author:  " << author() << std::endl;
	log::debug() << "  license: " << license() << std::endl;
	log::debug() << "  summary: " << summary() << std::endl;
	log::debug() << "  version: " << version() << std::endl;
}

JsPlugin::~JsPlugin()
{
	for (auto &timer : m_timers) {
		timer->stop();
	}
}

void JsPlugin::addTimer(std::shared_ptr<Timer> timer) noexcept
{
	std::weak_ptr<Timer> ptr(timer);

	/*
	 * These signals are called from the Timer thread and are transmitted to irccd so that it can
	 * calls appropriate timer functions.
	 */
	timer->onSignal.connect([this, ptr] () {
		auto timer = ptr.lock();

		if (timer) {
			onTimerSignal(move(timer));
		}
	});
	timer->onEnd.connect([this, ptr] () {
		auto timer = ptr.lock();

		if (timer) {
			onTimerEnd(move(timer));
		}
	});

	m_timers.insert(move(timer));
}

void JsPlugin::removeTimer(const std::shared_ptr<Timer> &timer) noexcept
{
	duk::StackAssert sa(m_context);

	/* Remove the JavaScript function */
	duk::push(m_context, duk::Null{});
	duk::putGlobal(m_context, "\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));

	/* Remove from list */
	m_timers.erase(timer);
}

void JsPlugin::onChannelMode(const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &mode,
			     const std::string &arg)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, mode);
	duk::push(m_context, arg);
	call("onChannelMode", 5);
}

void JsPlugin::onChannelNotice(const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &notice)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, notice);
	call("onChannelNotice", 4);
}

void JsPlugin::onCommand(const std::shared_ptr<Server> &server,
			 const std::string &origin,
			 const std::string &channel,
			 const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, message);
	call("onCommand", 4);
}

void JsPlugin::onConnect(const std::shared_ptr<Server> &server)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	call("onConnect", 1);
}

void JsPlugin::onInvite(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	call("onInvite", 3);
}

void JsPlugin::onJoin(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	call("onJoin", 3);
}

void JsPlugin::onKick(const std::shared_ptr<Server> &server,
		      const std::string &origin,
		      const std::string &channel,
		      const std::string &target,
		      const std::string &reason)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, target);
	duk::push(m_context, reason);
	call("onKick", 5);
}

void JsPlugin::onLoad()
{
	duk::StackAssert sa(m_context);

	call("onLoad", 0);
}

void JsPlugin::onMessage(const std::shared_ptr<Server> &server,
			 const std::string &origin,
			 const std::string &channel,
			 const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, message);
	call("onMessage", 4);
}

void JsPlugin::onMe(const std::shared_ptr<Server> &server,
		    const std::string &origin,
		    const std::string &channel,
		    const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, message);
	call("onMe", 4);
}

void JsPlugin::onMode(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, mode);
	call("onMode", 3);
}

void JsPlugin::onNames(const std::shared_ptr<Server> &server, const std::string &channel, const std::vector<std::string> &names)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, channel);
	duk::push(m_context, names);
	call("onNames", 3);
}

void JsPlugin::onNick(const std::shared_ptr<Server> &server, const std::string &oldnick, const std::string &newnick)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, oldnick);
	duk::push(m_context, newnick);
	call("onNick", 3);
}

void JsPlugin::onNotice(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, notice);
	call("onNotice", 3);
}

void JsPlugin::onPart(const std::shared_ptr<Server> &server,
		      const std::string &origin,
		      const std::string &channel,
		      const std::string &reason)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, reason);
	call("onPart", 4);
}

void JsPlugin::onQuery(const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, message);
	call("onQuery", 3);
}

void JsPlugin::onQueryCommand(const std::shared_ptr<Server> &server,
			      const std::string &origin,
			      const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, message);
	call("onQueryCommand", 3);
}

void JsPlugin::onReload()
{
	duk::StackAssert sa(m_context);

	call("onReload");
}

void JsPlugin::onTopic(const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &channel,
		       const std::string &topic)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	duk::push(m_context, topic);
	call("onTopic", 4);
}

void JsPlugin::onUnload()
{
	duk::StackAssert sa(m_context);

	call("onUnload");
}

void JsPlugin::onWhois(const std::shared_ptr<Server> &server, const ServerWhois &whois)
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

} // !irccd
