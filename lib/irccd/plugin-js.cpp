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
#include "irccd.hpp"
#include "logger.hpp"
#include "mod-server.hpp"
#include "plugin-js.hpp"
#include "service-module.hpp"
#include "timer.hpp"

namespace irccd {

void JsPlugin::call(const std::string &name, unsigned nargs)
{
	duk::getGlobal<void>(m_context, name);

	if (duk::type(m_context, -1) == DUK_TYPE_UNDEFINED)
		// Function not defined, remove the undefined value and all arguments.
		duk::pop(m_context, nargs + 1);
	else {
		// Call the function and discard the result.
		duk::insert(m_context, -nargs - 1);

		if (duk::pcall(m_context, nargs) != 0) {
			auto error = duk::error(m_context, -1);

			duk::pop(m_context);

			throw error;
		} else
			duk::pop(m_context);
	}
}

void JsPlugin::putModules(Irccd &irccd)
{
	for (const auto &module : irccd.moduleService().modules())
		module->load(irccd, *this);
}

void JsPlugin::putVars()
{
	duk::StackAssert sa(m_context);

	duk::putGlobal(m_context, "\xff""\xff""plugin", duk::RawPointer<JsPlugin>{this});
	duk::putGlobal(m_context, "\xff""\xff""name", name());
	duk::putGlobal(m_context, "\xff""\xff""path", path());
}

void JsPlugin::putPath(const std::string &varname, const std::string &append, path::Path type)
{
	duk::StackAssert sa(m_context);

	bool found = true;
	std::string foundpath;

	// Use the first existing directory available.
	for (const auto &p : path::list(type)) {
		foundpath = path::clean(p + append);

		if (fs::exists(foundpath)) {
			found = true;
			break;
		}
	}

	// Use the system as default.
	if (!found)
		foundpath = path::clean(path::get(type, path::OwnerSystem) + append);

	duk::getGlobal<void>(m_context, "Irccd");
	duk::getProperty<void>(m_context, -1, "Plugin");
	duk::putProperty(m_context, -1, varname, foundpath);
	duk::pop(m_context, 2);
}

void JsPlugin::putConfig(const PluginConfig &config)
{
	duk::StackAssert sa(m_context);

	// TODO: override dataPath, configPath, cachePath.
	// TODO: verify more that these values still exist.

	// Store plugin configuration into Irccd.Plugin.config.
	duk::getGlobal<void>(m_context, "Irccd");
	duk::getProperty<void>(m_context, -1, "Plugin");
	duk::getProperty<void>(m_context, -1, "config");

	for (const auto &pair : config)
		duk::putProperty(m_context, -1, pair.first, pair.second);

	duk::putProperty(m_context, -2, "config");
	duk::pop(m_context, 2);
}

void JsPlugin::putFormats()
{
	duk::StackAssert sa(m_context);

	duk::getGlobal<void>(m_context, "Irccd");
	duk::getProperty<void>(m_context, -1, "Plugin");
	duk::getProperty<void>(m_context, -1, "format");

	for (const auto &pair : formats())
		duk::putProperty(m_context, -1, pair.first, pair.second);

	duk::pop(m_context, 3);
}

JsPlugin::JsPlugin(std::string name, std::string path, const PluginConfig &config)
	: Plugin(name, path, config)
{
}

void JsPlugin::onChannelMode(Irccd &,
			     const std::shared_ptr<Server> &server,
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

void JsPlugin::onChannelNotice(Irccd &,
			       const std::shared_ptr<Server> &server,
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

void JsPlugin::onCommand(Irccd &,
			 const std::shared_ptr<Server> &server,
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

void JsPlugin::onConnect(Irccd &, const std::shared_ptr<Server> &server)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	call("onConnect", 1);
}

void JsPlugin::onInvite(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	call("onInvite", 3);
}

void JsPlugin::onJoin(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, channel);
	call("onJoin", 3);
}

void JsPlugin::onKick(Irccd &,
		      const std::shared_ptr<Server> &server,
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

void JsPlugin::onLoad(Irccd &irccd)
{
	duk::StackAssert sa(m_context);

	/*
	 * Duktape currently emit useless warnings when a file do
	 * not exists so we do a homemade access.
	 */
#if defined(HAVE_STAT)
	struct stat st;

	if (::stat(path().c_str(), &st) < 0) {
		throw std::runtime_error(std::strerror(errno));
	}
#endif

	/*
	 * dataPath: DATA + plugin/name (e.g ~/.local/share/irccd/plugins/<name>/)
	 * configPath: CONFIG + plugin/name (e.g ~/.config/irccd/plugin/<name>/)
	 */
	putModules(irccd);
	putVars();
	putPath("dataPath", "plugin/" + name(), path::PathData);
	putPath("configPath", "plugin/" + name(), path::PathConfig);
	putPath("cachePath", "plugin/" + name(), path::PathCache);

	// Try to load the file (does not call onLoad yet).
	if (duk::pevalFile(m_context, path()) != 0)
		throw duk::error(m_context, -1);

	duk::pop(m_context);

	putConfig(config());
	putFormats();

	// Read metadata .
	duk::getGlobal<void>(m_context, "info");

	if (duk::type(m_context, -1) == DUK_TYPE_OBJECT) {
		setAuthor(duk::optionalProperty<std::string>(m_context, -1, "author", author()));
		setLicense(duk::optionalProperty<std::string>(m_context, -1, "license", license()));
		setSummary(duk::optionalProperty<std::string>(m_context, -1, "summary", summary()));
		setVersion(duk::optionalProperty<std::string>(m_context, -1, "version", version()));
	}

	duk::pop(m_context);

	call("onLoad", 0);
}

void JsPlugin::onMessage(Irccd &,
			 const std::shared_ptr<Server> &server,
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

void JsPlugin::onMe(Irccd &,
		    const std::shared_ptr<Server> &server,
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

void JsPlugin::onMode(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, mode);
	call("onMode", 3);
}

void JsPlugin::onNames(Irccd &, const std::shared_ptr<Server> &server, const std::string &channel, const std::vector<std::string> &names)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, channel);
	duk::push(m_context, names);
	call("onNames", 3);
}

void JsPlugin::onNick(Irccd &, const std::shared_ptr<Server> &server, const std::string &oldnick, const std::string &newnick)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, oldnick);
	duk::push(m_context, newnick);
	call("onNick", 3);
}

void JsPlugin::onNotice(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, notice);
	call("onNotice", 3);
}

void JsPlugin::onPart(Irccd &,
		      const std::shared_ptr<Server> &server,
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

void JsPlugin::onQuery(Irccd &,
		       const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, message);
	call("onQuery", 3);
}

void JsPlugin::onQueryCommand(Irccd &,
			      const std::shared_ptr<Server> &server,
			      const std::string &origin,
			      const std::string &message)
{
	duk::StackAssert sa(m_context);

	duk::push(m_context, duk::Shared<Server>{server});
	duk::push(m_context, origin);
	duk::push(m_context, message);
	call("onQueryCommand", 3);
}

void JsPlugin::onReload(Irccd &)
{
	duk::StackAssert sa(m_context);

	call("onReload");
}

void JsPlugin::onTopic(Irccd &,
		       const std::shared_ptr<Server> &server,
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

void JsPlugin::onUnload(Irccd &irccd)
{
	duk::StackAssert sa(m_context);

	call("onUnload");

	for (const auto &module : irccd.moduleService().modules())
		module->unload(irccd, *this);
}

void JsPlugin::onWhois(Irccd &, const std::shared_ptr<Server> &server, const ServerWhois &whois)
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
