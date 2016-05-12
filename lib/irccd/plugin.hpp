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
 * \class Plugin
 * \brief JavaScript plugin
 *
 * A plugin is identified by name and can be loaded and unloaded
 * at runtime.
 */
class Plugin {
private:
	// Plugin information
	std::string m_name;
	std::string m_path;

	// Metadata
	std::string m_author{"unknown"};
	std::string m_license{"unknown"};
	std::string m_summary{"unknown"};
	std::string m_version{"unknown"};

	PluginConfig m_config;

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
	inline Plugin(std::string name, std::string path, PluginConfig config = PluginConfig()) noexcept
		: m_name(std::move(name))
		, m_path(std::move(path))
		, m_config(std::move(config))
	{
	}

	/**
	 * Temporary, close all timers.
	 */
	virtual ~Plugin() = default;

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
	 * On channel message. This event will call onMessage or
	 * onCommand if the messages starts with the command character
	 * plus the plugin name.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	virtual void onCommand(const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &message)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)message;
	}

	/**
	 * On successful connection.
	 *
	 * \param server the server
	 */
	virtual void onConnect(const std::shared_ptr<Server> &server)
	{
		(void)server;
	}

	/**
	 * On channel mode.
	 *
	 * \param server the server
	 * \param origin the ouser who has changed the mode
	 * \param channel the channel
	 * \param mode the mode
	 * \param arg the optional mode argument
	 */
	virtual void onChannelMode(const std::shared_ptr<Server> &server,
				   const std::string &origin,
				   const std::string &channel,
				   const std::string &mode,
				   const std::string &arg)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)mode;
		(void)arg;
	}

	/**
	 * On a channel notice.
	 *
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param channel on which channel
	 * \param notice the message
	 */
	virtual void onChannelNotice(const std::shared_ptr<Server> &server,
				     const std::string &origin,
				     const std::string &channel,
				     const std::string &notice)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)notice;
	}

	/**
	 * On invitation.
	 *
	 * \param server the server
	 * \param origin the user who invited you
	 * \param channel the channel
	 */
	virtual void onInvite(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
	{
		(void)server;
		(void)origin;
		(void)channel;
	}

	/**
	 * On join.
	 *
	 * \param server the server
	 * \param origin the user who joined
	 * \param channel the channel
	 */
	virtual void onJoin(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
	{
		(void)server;
		(void)origin;
		(void)channel;
	}

	/**
	 * On kick.
	 *
	 * \param server the server
	 * \param origin the user who kicked the target
	 * \param channel the channel
	 * \param target the kicked target
	 * \param reason the optional reason
	 */
	virtual void onKick(const std::shared_ptr<Server> &server,
			    const std::string &origin,
			    const std::string &channel,
			    const std::string &target,
			    const std::string &reason)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)target;
		(void)reason;
	}

	/**
	 * On load.
	 */
	virtual void onLoad()
	{
	}

	/**
	 * On channel message.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	virtual void onMessage(const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &message)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)message;
	}

	/**
	 * On CTCP Action.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel (may also be your nickname)
	 * \param message the message
	 */
	virtual void onMe(const std::shared_ptr<Server> &server,
			  const std::string &origin,
			  const std::string &channel,
			  const std::string &message)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)message;
	}

	/**
	 * On user mode change.
	 *
	 * \param server the server
	 * \param origin the person who changed the mode
	 * \param mode the new mode
	 */
	virtual void onMode(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
	{
		(void)server;
		(void)origin;
		(void)mode;
	}

	/**
	 * On names listing.
	 *
	 * \param server the server
	 * \param channel the channel
	 * \param list the list of nicknames
	 */
	virtual void onNames(const std::shared_ptr<Server> &server, const std::string &channel, const std::vector<std::string> &list)
	{
		(void)server;
		(void)channel;
		(void)list;
	}

	/**
	 * On nick change.
	 *
	 * \param server the server
	 * \param origin the user that changed its nickname
	 * \param nick the new nickname
	 */
	virtual void onNick(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick)
	{
		(void)server;
		(void)origin;
		(void)nick;
	}

	/**
	 * On user notice.
	 *
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param notice the notice
	 */
	virtual void onNotice(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
	{
		(void)server;
		(void)origin;
		(void)notice;
	}

	/**
	 * On part.
	 *
	 * \param server the server
	 * \param origin the user who left
	 * \param channel the channel
	 * \param reason the optional reason
	 */
	virtual void onPart(const std::shared_ptr<Server> &server,
			    const std::string &origin,
			    const std::string &channel,
			    const std::string &reason)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)reason;
	}

	/**
	 * On user query.
	 *
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	virtual void onQuery(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message)
	{
		(void)server;
		(void)origin;
		(void)message;
	}

	/**
	 * On user query command.
	 *
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	virtual void onQueryCommand(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message)
	{
		(void)server;
		(void)origin;
		(void)message;
	}

	/**
	 * On reload.
	 */
	virtual void onReload()
	{
	}

	/**
	 * On topic change.
	 *
	 * \param server the server
	 * \param origin the user who sent the topic
	 * \param channel the channel
	 * \param topic the new topic
	 */
	virtual void onTopic(const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &topic)
	{
		(void)server;
		(void)origin;
		(void)channel;
		(void)topic;
	}

	/**
	 * On unload.
	 */
	virtual void onUnload()
	{
	}

	/**
	 * On whois information.
	 *
	 * \param server the server
	 * \param info the info
	 */
	virtual void onWhois(const std::shared_ptr<Server> &server, const ServerWhois &info)
	{
		(void)server;
		(void)info;
	}
};

} // !irccd

#endif // !IRCCD_PLUGIN_HPP
