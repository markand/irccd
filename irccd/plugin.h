/*
 * plugin.h -- irccd JavaScript plugin interface
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

#ifndef _IRCCD_PLUGIN_H_
#define _IRCCD_PLUGIN_H_

/**
 * @file Plugin.h
 * @brief Irccd plugins
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <path.h>
#include <signals.h>

#include "js.h"
#include "timer.h"

namespace irccd {

class Server;
class ServerWhois;

/**
 * @class PluginInfo
 * @brief Plugin information
 */
class PluginInfo {
public:
	std::string name;		//!< plugin name (from file on disk)
	std::string parent;		//!< parent directory
	std::string path;		//!< full path to the plugin file

	/* Metadata */
	std::string author{"unknown"};	//!< plugin author
	std::string license{"unknown"};	//!< plugin license
	std::string summary{"unknown"};	//!< short plugin description
	std::string version{"unknown"};	//!< plugin version
};

/**
 * Configuration map extract from config file.
 */
using PluginConfig = std::unordered_map<std::string, std::string>;

/**
 * Timers that a plugin owns.
 */
using PluginTimers = std::unordered_set<std::shared_ptr<Timer>>;

/**
 * @class Plugin
 * @brief JavaScript plugin
 *
 * A plugin is identified by name and can be loaded and unloaded
 * at runtime.
 */
class Plugin {
public:
	/**
	 * Signal: onTimerSignal
	 * ------------------------------------------------
	 *
	 * When a timer expires.
	 *
	 * Arguments:
	 * - the timer object
	 */
	Signal<std::shared_ptr<Timer>> onTimerSignal;

	/**
	 * Signal: onTimerEnd
	 * ------------------------------------------------
	 *
	 * When a timer is finished.
	 *
	 * Arguments:
	 * - the timer object
	 */
	Signal<std::shared_ptr<Timer>> onTimerEnd;

private:
	/* JavaScript context */
	js::Context m_context;

	/* Plugin info and its timers */
	PluginInfo m_info;
	PluginTimers m_timers;

	/* Private helpers */
	void call(const std::string &name, unsigned nargs = 0);
	void putVars();
	void putPath(const std::string &varname, const std::string &append, path::Path type);
	void putPaths();
	void putConfig(const PluginConfig &config);

public:
	/**
	 * Correct constructor.
	 *
	 * @param name the plugin id
	 * @param path the fully resolved path to the plugin
	 * @param config the plugin configuration
	 * @throws std::runtime_error on errors
	 */
	Plugin(std::string name, std::string path, const PluginConfig &config = PluginConfig());

	/**
	 * Get the plugin information.
	 */
	const PluginInfo &info() const;

	/**
	 * Add a timer to the plugin.
	 *
	 * @param timer the timer to add
	 */
	void addTimer(std::shared_ptr<Timer> timer) noexcept;

	/**
	 * Remove a timer from a plugin.
	 *
	 * @param timer
	 */
	void removeTimer(const std::shared_ptr<Timer> &timer) noexcept;

	/**
	 * Access the Duktape context.
	 *
	 * @return the context
	 */
	inline js::Context &context() noexcept
	{
		return m_context;
	}

	/**
	 * On channel message. This event will call onMessage or
	 * onCommand if the messages starts with the command character
	 * plus the plugin name.
	 *
	 * @param server the server
	 * @param origin the user who sent the message
	 * @param channel the channel
	 * @param message the message or command
	 */
	void onCommand(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message);

	/**
	 * On successful connection.
	 *
	 * @param server the server
	 */
	void onConnect(std::shared_ptr<Server> server);

	/**
	 * On channel mode.
	 *
	 * @param server the server
	 * @param origin the ouser who has changed the mode
	 * @param channel the channel
	 * @param mode the mode
	 * @param arg the optional mode argument
	 */
	void onChannelMode(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string mode, std::string arg);

	/**
	 * On a channel notice.
	 *
	 * @param server the server
	 * @param origin the user who sent the notice
	 * @param channel on which channel
	 * @param notice the message
	 */
	void onChannelNotice(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string notice);

	/**
	 * On invitation.
	 *
	 * @param server the server
	 * @param origin the user who invited you
	 * @param channel the channel
	 */
	void onInvite(std::shared_ptr<Server> server, std::string origin, std::string channel);

	/**
	 * On join.
	 *
	 * @param server the server
	 * @param origin the user who joined
	 * @param channel the channel
	 */
	void onJoin(std::shared_ptr<Server> server, std::string origin, std::string channel);

	/**
	 * On kick.
	 *
	 * @param server the server
	 * @param origin the user who kicked the target
	 * @param channel the channel
	 * @param target the kicked target
	 * @param reason the optional reason
	 */
	void onKick(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string target, std::string reason);

	/**
	 * On load.
	 */
	void onLoad();

	/**
	 * On channel message.
	 *
	 * @param server the server
	 * @param origin the user who sent the message
	 * @param channel the channel
	 * @param message the message or command
	 */
	void onMessage(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message);

	/**
	 * On CTCP Action.
	 *
	 * @param server the server
	 * @param origin the user who sent the message
	 * @param channel the channel (may also be your nickname)
	 * @param message the message
	 */
	void onMe(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message);

	/**
	 * On user mode change.
	 *
	 * @param server the server
	 * @param origin the person who changed the mode
	 * @param mode the new mode
	 */
	void onMode(std::shared_ptr<Server> server, std::string origin, std::string mode);

	/**
	 * On names listing.
	 *
	 * @param server the server
	 * @param channel the channel
	 * @param list the list of nicknames
	 */
	void onNames(std::shared_ptr<Server> server, std::string channel, std::vector<std::string> list);

	/**
	 * On nick change.
	 *
	 * @param server the server
	 * @param origin the user that changed its nickname
	 * @param nick the new nickname
	 */
	void onNick(std::shared_ptr<Server> server, std::string origin, std::string nick);

	/**
	 * On user notice.
	 *
	 * @param server the server
	 * @param origin the user who sent the notice
	 * @param notice the notice
	 */
	void onNotice(std::shared_ptr<Server> server, std::string origin, std::string notice);

	/**
	 * On part.
	 *
	 * @param server the server
	 * @param origin the user who left
	 * @param channel the channel
	 * @param reason the optional reason
	 */
	void onPart(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string reason);

	/**
	 * On user query.
	 *
	 * @param server the server
	 * @param origin the user who sent the query
	 * @param message the message
	 */
	void onQuery(std::shared_ptr<Server> server, std::string origin, std::string message);

	/**
	 * On user query command.
	 *
	 * @param server the server
	 * @param origin the user who sent the query
	 * @param message the message
	 */
	void onQueryCommand(std::shared_ptr<Server> server, std::string origin, std::string message);

	/**
	 * On reload.
	 */
	void onReload();

	/**
	 * On topic change.
	 *
	 * @param server the server
	 * @param origin the user who sent the topic
	 * @param channel the channel
	 * @param topic the new topic
	 */
	void onTopic(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string topic);

	/**
	 * On unload.
	 */
	void onUnload();

	/**
	 * On whois information.
	 *
	 * @param server the server
	 * @param info the info
	 */
	void onWhois(std::shared_ptr<Server> server, ServerWhois info);
};

namespace js {

template <>
class TypeInfo<PluginInfo> {
public:
	static void push(Context &ctx, const PluginInfo &info);
};

} // !js

} // !irccd

#endif // !_IRCCD_PLUGIN_H_
