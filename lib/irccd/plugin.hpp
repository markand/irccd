/*
 * plugin.hpp -- irccd JavaScript plugin interface
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

#ifndef IRCCD_PLUGIN_HPP
#define IRCCD_PLUGIN_HPP

/**
 * \file plugin.hpp
 * \brief Irccd plugins
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "js.hpp"
#include "path.hpp"
#include "signals.hpp"
#include "timer.hpp"

namespace irccd {

class Server;
class ServerWhois;

/**
 * Configuration map extract from config file.
 */
using PluginConfig = std::unordered_map<std::string, std::string>;

/**
 * Timers that a plugin owns.
 */
using PluginTimers = std::unordered_set<std::shared_ptr<Timer>>;

/**
 * \class Plugin
 * \brief JavaScript plugin
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
	// Plugin information
	std::string m_name;
	std::string m_path;

	// Metadata
	std::string m_author{"unknown"};
	std::string m_license{"unknown"};
	std::string m_summary{"unknown"};
	std::string m_version{"unknown"};

	// JavaScript context
	duk::Context m_context;

	// Plugin info and its timers
	PluginTimers m_timers;

	// Private helpers
	void call(const std::string &name, unsigned nargs = 0);
	void putVars();
	void putPath(const std::string &varname, const std::string &append, path::Path type);
	void putPaths();
	void putConfig(const PluginConfig &config);

public:
	/**
	 * Find plugin in standard paths.
	 *
	 * \param name the plugin name
	 * \param config the optional configuration
	 */
	static std::shared_ptr<Plugin> find(const std::string &name, const PluginConfig &config = PluginConfig());

	/**
	 * Constructor.
	 *
	 * \param name the plugin id
	 * \param path the fully resolved path to the plugin
	 * \param config the plugin configuration
	 * \throws std::runtime_error on errors
	 */
	Plugin(std::string name, std::string path, const PluginConfig &config = PluginConfig());

	/**
	 * Temporary, close all timers.
	 */
	~Plugin();

	/**
	 * Get the plugin name.
	 *
	 * \return the plugin name
	 */
	inline const std::string &name() const noexcept
	{
		return m_name;
	}

	/**
	 * Get the plugin path.
	 *
	 * \return the plugin path
	 * \note some plugins may not exist on the disk
	 */
	inline const std::string &path() const noexcept
	{
		return m_path;
	}

	/**
	 * Get the author.
	 *
	 * \return the author
	 */
	inline const std::string &author() const noexcept
	{
		return m_author;
	}

	/**
	 * Set the author.
	 *
	 * \param author the author
	 */
	inline void setAuthor(std::string author) noexcept
	{
		m_author = std::move(author);
	}

	/**
	 * Get the license.
	 *
	 * \return the license
	 */
	inline const std::string &license() const noexcept
	{
		return m_license;
	}

	/**
	 * Set the license.
	 *
	 * \param license the license
	 */
	inline void setLicense(std::string license) noexcept
	{
		m_license = std::move(license);
	}

	/**
	 * Get the summary.
	 *
	 * \return the summary
	 */
	inline const std::string &summary() const noexcept
	{
		return m_summary;
	}

	/**
	 * Set the summary.
	 *
	 * \param summary the summary
	 */
	inline void setSummary(std::string summary) noexcept
	{
		m_summary = std::move(summary);
	}

	/**
	 * Get the version.
	 *
	 * \return the version
	 */
	inline const std::string &version() const noexcept
	{
		return m_version;
	}

	/**
	 * Set the version.
	 *
	 * \param version the version
	 */
	inline void setVersion(std::string version) noexcept
	{
		m_version = std::move(version);
	}

	/**
	 * Add a timer to the plugin.
	 *
	 * \param timer the timer to add
	 */
	void addTimer(std::shared_ptr<Timer> timer) noexcept;

	/**
	 * Remove a timer from a plugin.
	 *
	 * \param timer
	 */
	void removeTimer(const std::shared_ptr<Timer> &timer) noexcept;

	/**
	 * Access the Duktape context.
	 *
	 * \return the context
	 */
	inline duk::Context &context() noexcept
	{
		return m_context;
	}

	/**
	 * On channel message. This event will call onMessage or
	 * onCommand if the messages starts with the command character
	 * plus the plugin name.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	void onCommand(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message);

	/**
	 * On successful connection.
	 *
	 * \param server the server
	 */
	void onConnect(std::shared_ptr<Server> server);

	/**
	 * On channel mode.
	 *
	 * \param server the server
	 * \param origin the ouser who has changed the mode
	 * \param channel the channel
	 * \param mode the mode
	 * \param arg the optional mode argument
	 */
	void onChannelMode(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string mode, std::string arg);

	/**
	 * On a channel notice.
	 *
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param channel on which channel
	 * \param notice the message
	 */
	void onChannelNotice(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string notice);

	/**
	 * On invitation.
	 *
	 * \param server the server
	 * \param origin the user who invited you
	 * \param channel the channel
	 */
	void onInvite(std::shared_ptr<Server> server, std::string origin, std::string channel);

	/**
	 * On join.
	 *
	 * \param server the server
	 * \param origin the user who joined
	 * \param channel the channel
	 */
	void onJoin(std::shared_ptr<Server> server, std::string origin, std::string channel);

	/**
	 * On kick.
	 *
	 * \param server the server
	 * \param origin the user who kicked the target
	 * \param channel the channel
	 * \param target the kicked target
	 * \param reason the optional reason
	 */
	void onKick(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string target, std::string reason);

	/**
	 * On load.
	 */
	void onLoad();

	/**
	 * On channel message.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	void onMessage(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message);

	/**
	 * On CTCP Action.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel (may also be your nickname)
	 * \param message the message
	 */
	void onMe(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string message);

	/**
	 * On user mode change.
	 *
	 * \param server the server
	 * \param origin the person who changed the mode
	 * \param mode the new mode
	 */
	void onMode(std::shared_ptr<Server> server, std::string origin, std::string mode);

	/**
	 * On names listing.
	 *
	 * \param server the server
	 * \param channel the channel
	 * \param list the list of nicknames
	 */
	void onNames(std::shared_ptr<Server> server, std::string channel, std::vector<std::string> list);

	/**
	 * On nick change.
	 *
	 * \param server the server
	 * \param origin the user that changed its nickname
	 * \param nick the new nickname
	 */
	void onNick(std::shared_ptr<Server> server, std::string origin, std::string nick);

	/**
	 * On user notice.
	 *
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param notice the notice
	 */
	void onNotice(std::shared_ptr<Server> server, std::string origin, std::string notice);

	/**
	 * On part.
	 *
	 * \param server the server
	 * \param origin the user who left
	 * \param channel the channel
	 * \param reason the optional reason
	 */
	void onPart(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string reason);

	/**
	 * On user query.
	 *
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	void onQuery(std::shared_ptr<Server> server, std::string origin, std::string message);

	/**
	 * On user query command.
	 *
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	void onQueryCommand(std::shared_ptr<Server> server, std::string origin, std::string message);

	/**
	 * On reload.
	 */
	void onReload();

	/**
	 * On topic change.
	 *
	 * \param server the server
	 * \param origin the user who sent the topic
	 * \param channel the channel
	 * \param topic the new topic
	 */
	void onTopic(std::shared_ptr<Server> server, std::string origin, std::string channel, std::string topic);

	/**
	 * On unload.
	 */
	void onUnload();

	/**
	 * On whois information.
	 *
	 * \param server the server
	 * \param info the info
	 */
	void onWhois(std::shared_ptr<Server> server, ServerWhois info);
};

} // !irccd

#endif // !IRCCD_PLUGIN_HPP
