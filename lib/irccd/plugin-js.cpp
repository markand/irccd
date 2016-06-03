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
	duk_get_global_string(m_context, name.c_str());

	if (duk_get_type(m_context, -1) == DUK_TYPE_UNDEFINED)
		// Function not defined, remove the undefined value and all arguments.
		duk_pop_n(m_context, nargs + 1);
	else {
		// Call the function and discard the result.
		duk_insert(m_context, -nargs - 1);

		if (duk_pcall(m_context, nargs) != 0)
			throw duk_exception(m_context, -1, true);

		duk_pop(m_context);
	}
}

void JsPlugin::putModules(Irccd &irccd)
{
	for (const auto &module : irccd.moduleService().modules())
		module->load(irccd, *this);
}

void JsPlugin::putVars()
{
	StackAssert sa(m_context);

	duk_push_pointer(m_context, this);
	duk_get_global_string(m_context, "\xff""\xff""plugin");
	duk_push_stdstring(m_context, name());
	duk_put_global_string(m_context, "\xff""\xff""name");
	duk_push_stdstring(m_context, path());
	duk_put_global_string(m_context, "\xff""\xff""path");
}

void JsPlugin::putPath(const std::string &varname, const std::string &append, path::Path type)
{
	StackAssert sa(m_context);

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

	duk_get_global_string(m_context, "Irccd");
	duk_get_prop_string(m_context, -2, "Plugin");
	duk_push_stdstring(m_context, foundpath);
	duk_put_prop_string(m_context, -2, varname.c_str());
	duk_pop_n(m_context, 2);
}

void JsPlugin::putConfig(const PluginConfig &config)
{
	StackAssert sa(m_context);

	// TODO: override dataPath, configPath, cachePath.
	// TODO: verify more that these values still exist.

	// Store plugin configuration into Irccd.Plugin.config.
	duk_get_global_string(m_context, "Irccd");
	duk_get_prop_string(m_context, -1, "Plugin");
	duk_get_prop_string(m_context, -1, "config");

	for (const auto &pair : config) {
		duk_push_stdstring(m_context, pair.second);
		duk_put_prop_string(m_context, -2, pair.first.c_str());
	}

	duk_put_prop_string(m_context, -2, "config");
	duk_pop_n(m_context, 2);
}

void JsPlugin::putFormats()
{
	StackAssert sa(m_context);

	duk_get_global_string(m_context, "Irccd");
	duk_get_prop_string(m_context, -1, "Plugin");
	duk_get_prop_string(m_context, -1, "format");

	for (const auto &pair : formats()) {
		duk_push_stdstring(m_context, pair.second);
		duk_put_prop_string(m_context, -1, pair.first.c_str());
	}

	duk_pop_n(m_context, 3);
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
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, mode);
	duk_push_stdstring(m_context, arg);
	call("onChannelMode", 5);
}

void JsPlugin::onChannelNotice(Irccd &,
			       const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &notice)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, notice);
	call("onChannelNotice", 4);
}

void JsPlugin::onCommand(Irccd &,
			 const std::shared_ptr<Server> &server,
			 const std::string &origin,
			 const std::string &channel,
			 const std::string &message)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, message);
	call("onCommand", 4);
}

void JsPlugin::onConnect(Irccd &, const std::shared_ptr<Server> &server)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	call("onConnect", 1);
}

void JsPlugin::onInvite(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	call("onInvite", 3);
}

void JsPlugin::onJoin(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	call("onJoin", 3);
}

void JsPlugin::onKick(Irccd &,
		      const std::shared_ptr<Server> &server,
		      const std::string &origin,
		      const std::string &channel,
		      const std::string &target,
		      const std::string &reason)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, target);
	duk_push_stdstring(m_context, reason);
	call("onKick", 5);
}

void JsPlugin::onLoad(Irccd &irccd)
{
	StackAssert sa(m_context);

	/*
	 * Duktape currently emit useless warnings when a file do
	 * not exists so we do a homemade access.
	 */
#if defined(HAVE_STAT)
	struct stat st;

	if (::stat(path().c_str(), &st) < 0)
		throw std::runtime_error(std::strerror(errno));
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
	if (duk_peval_file(m_context, path().c_str()) != 0)
		throw duk_exception(m_context, -1, true);

	duk_pop(m_context);

	putConfig(config());
	putFormats();

	// Read metadata .
	duk_get_global_string(m_context, "info");

	if (duk_get_type(m_context, -1) == DUK_TYPE_OBJECT) {
		// TODO: bring back
#if 0
		setAuthor(optionalProperty<std::string>(m_context, -1, "author", author()));
		setLicense(optionalProperty<std::string>(m_context, -1, "license", license()));
		setSummary(optionalProperty<std::string>(m_context, -1, "summary", summary()));
		setVersion(optionalProperty<std::string>(m_context, -1, "version", version()));
#endif
	}

	duk_pop(m_context);
	call("onLoad", 0);
}

void JsPlugin::onMessage(Irccd &,
			 const std::shared_ptr<Server> &server,
			 const std::string &origin,
			 const std::string &channel,
			 const std::string &message)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, message);
	call("onMessage", 4);
}

void JsPlugin::onMe(Irccd &,
		    const std::shared_ptr<Server> &server,
		    const std::string &origin,
		    const std::string &channel,
		    const std::string &message)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, message);
	call("onMe", 4);
}

void JsPlugin::onMode(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, mode);
	call("onMode", 3);
}

void JsPlugin::onNames(Irccd &, const std::shared_ptr<Server> &server, const std::string &channel, const std::vector<std::string> &names)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, channel);
	// TODO
	// duk_push_stdstring(m_context, names);
	call("onNames", 3);
}

void JsPlugin::onNick(Irccd &, const std::shared_ptr<Server> &server, const std::string &oldnick, const std::string &newnick)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, oldnick);
	duk_push_stdstring(m_context, newnick);
	call("onNick", 3);
}

void JsPlugin::onNotice(Irccd &, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, notice);
	call("onNotice", 3);
}

void JsPlugin::onPart(Irccd &,
		      const std::shared_ptr<Server> &server,
		      const std::string &origin,
		      const std::string &channel,
		      const std::string &reason)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, reason);
	call("onPart", 4);
}

void JsPlugin::onQuery(Irccd &,
		       const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &message)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, message);
	call("onQuery", 3);
}

void JsPlugin::onQueryCommand(Irccd &,
			      const std::shared_ptr<Server> &server,
			      const std::string &origin,
			      const std::string &message)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, message);
	call("onQueryCommand", 3);
}

void JsPlugin::onReload(Irccd &)
{
	StackAssert sa(m_context);

	call("onReload");
}

void JsPlugin::onTopic(Irccd &,
		       const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &channel,
		       const std::string &topic)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_stdstring(m_context, origin);
	duk_push_stdstring(m_context, channel);
	duk_push_stdstring(m_context, topic);
	call("onTopic", 4);
}

void JsPlugin::onUnload(Irccd &irccd)
{
	StackAssert sa(m_context);

	call("onUnload");

	for (const auto &module : irccd.moduleService().modules())
		module->unload(irccd, *this);
}

void JsPlugin::onWhois(Irccd &, const std::shared_ptr<Server> &server, const ServerWhois &whois)
{
	StackAssert sa(m_context);

	duk_push_server(m_context, server);
	duk_push_object(m_context);
	duk_push_stdstring(m_context, whois.nick);
	duk_put_prop_string(m_context, -2, "nickname");
	duk_push_stdstring(m_context, whois.user);
	duk_put_prop_string(m_context, -2, "username");
	duk_push_stdstring(m_context, whois.realname);
	duk_put_prop_string(m_context, -2, "realname");
	duk_push_stdstring(m_context, whois.host);
	duk_put_prop_string(m_context, -2, "host");
	// TODO
	// duk_put_prop_string(m_context, -2, "channels", whois.channels);
	call("onWhois", 2);
}

} // !irccd
