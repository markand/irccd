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
#include <vector>

#include "sysconfig.hpp"
#include "util.hpp"

namespace irccd {

class Irccd;
class Server;
class ServerWhois;

/**
 * \brief Configuration map extract from config file.
 */
using PluginConfig = std::unordered_map<std::string, std::string>;

/**
 * \brief Formats for plugins.
 */
using PluginFormats = std::unordered_map<std::string, std::string>;

/**
 * \class Plugin
 * \brief JavaScript plugin
 *
 * A plugin is identified by name and can be loaded and unloaded
 * at runtime.
 */
class Plugin : public std::enable_shared_from_this<Plugin> {
private:
	// Plugin information
	std::string m_name;
	std::string m_path;

	// Metadata
	std::string m_author{"unknown"};
	std::string m_license{"unknown"};
	std::string m_summary{"unknown"};
	std::string m_version{"unknown"};

	// Configuration and formats.
	PluginConfig m_config;
	PluginFormats m_formats;

public:
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
	 * Access the plugin configuration.
	 *
	 * \return the config
	 */
	inline const PluginConfig &config() const noexcept
	{
		return m_config;
	}

	/**
	 * Overloaded function.
	 *
	 * \return the config
	 */
	inline PluginConfig &config() noexcept
	{
		return m_config;
	}

	/**
	 * Set the configuration.
	 *
	 * \param config the configuration
	 */
	inline void setConfig(PluginConfig config) noexcept
	{
		m_config = std::move(config);
	}

	/**
	 * Access the plugin formats.
	 *
	 * \return the format
	 */
	inline const PluginFormats &formats() const noexcept
	{
		return m_formats;
	}

	/**
	 * Overloaded function.
	 *
	 * \return the formats
	 */
	inline PluginFormats &formats() noexcept
	{
		return m_formats;
	}

	/**
	 * Set the formats.
	 *
	 * \param formats the formats
	 */
	inline void setFormats(PluginFormats formats) noexcept
	{
		m_formats = std::move(formats);
	}

	/**
	 * On channel message. This event will call onMessage or
	 * onCommand if the messages starts with the command character
	 * plus the plugin name.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	virtual void onCommand(Irccd &irccd,
			       const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &message)
	{
		util::unused(irccd, server, origin, channel, message);
	}

	/**
	 * On successful connection.
	 *
	 * \param server the server
	 */
	virtual void onConnect(Irccd &irccd, const std::shared_ptr<Server> &server)
	{
		util::unused(irccd, server);
	}

	/**
	 * On channel mode.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the ouser who has changed the mode
	 * \param channel the channel
	 * \param mode the mode
	 * \param arg the optional mode argument
	 */
	virtual void onChannelMode(Irccd &irccd,
				   const std::shared_ptr<Server> &server,
				   const std::string &origin,
				   const std::string &channel,
				   const std::string &mode,
				   const std::string &arg)
	{
		util::unused(irccd, server, origin, channel, mode, arg);
	}

	/**
	 * On a channel notice.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param channel on which channel
	 * \param notice the message
	 */
	virtual void onChannelNotice(Irccd &irccd,
				     const std::shared_ptr<Server> &server,
				     const std::string &origin,
				     const std::string &channel,
				     const std::string &notice)
	{
		util::unused(irccd, server, origin, channel, notice);
	}

	/**
	 * On invitation.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who invited you
	 * \param channel the channel
	 */
	virtual void onInvite(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
	{
		util::unused(irccd, server, origin, channel);
	}

	/**
	 * On join.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who joined
	 * \param channel the channel
	 */
	virtual void onJoin(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
	{
		util::unused(irccd, server, origin, channel);
	}

	/**
	 * On kick.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who kicked the target
	 * \param channel the channel
	 * \param target the kicked target
	 * \param reason the optional reason
	 */
	virtual void onKick(Irccd &irccd, 
			    const std::shared_ptr<Server> &server,
			    const std::string &origin,
			    const std::string &channel,
			    const std::string &target,
			    const std::string &reason)
	{
		util::unused(irccd, server, origin, channel, target, reason);
	}

	/**
	 * On load.
	 *
	 * \param irccd the irccd instance
	 */
	virtual void onLoad(Irccd &irccd)
	{
		util::unused(irccd);
	}

	/**
	 * On channel message.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	virtual void onMessage(Irccd &irccd, 
			       const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &message)
	{
		util::unused(irccd, server, origin, channel, message);
	}

	/**
	 * On CTCP Action.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel (may also be your nickname)
	 * \param message the message
	 */
	virtual void onMe(Irccd &irccd, 
			  const std::shared_ptr<Server> &server,
			  const std::string &origin,
			  const std::string &channel,
			  const std::string &message)
	{
		util::unused(irccd, server, origin, channel, message);
	}

	/**
	 * On user mode change.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the person who changed the mode
	 * \param mode the new mode
	 */
	virtual void onMode(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
	{
		util::unused(irccd, server, origin, mode);
	}

	/**
	 * On names listing.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param channel the channel
	 * \param list the list of nicknames
	 */
	virtual void onNames(Irccd &irccd,
			     const std::shared_ptr<Server> &server,
			     const std::string &channel,
			     const std::vector<std::string> &list)
	{
		util::unused(irccd, server, channel, list);
	}

	/**
	 * On nick change.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user that changed its nickname
	 * \param nick the new nickname
	 */
	virtual void onNick(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick)
	{
		util::unused(irccd, server, origin, nick);
	}

	/**
	 * On user notice.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param notice the notice
	 */
	virtual void onNotice(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
	{
		util::unused(irccd, server, origin, notice);
	}

	/**
	 * On part.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who left
	 * \param channel the channel
	 * \param reason the optional reason
	 */
	virtual void onPart(Irccd &irccd, 
			    const std::shared_ptr<Server> &server,
			    const std::string &origin,
			    const std::string &channel,
			    const std::string &reason)
	{
		util::unused(irccd, server, origin, channel, reason);
	}

	/**
	 * On user query.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	virtual void onQuery(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message)
	{
		util::unused(irccd, server, origin, message);
	}

	/**
	 * On user query command.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	virtual void onQueryCommand(Irccd &irccd,
				    const std::shared_ptr<Server> &server,
				    const std::string &origin,
				    const std::string &message)
	{
		util::unused(irccd, server, origin, message);
	}

	/**
	 * On reload.
	 *
	 * \param irccd the irccd instance
	 */
	virtual void onReload(Irccd &irccd)
	{
		util::unused(irccd);
	}

	/**
	 * On topic change.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param origin the user who sent the topic
	 * \param channel the channel
	 * \param topic the new topic
	 */
	virtual void onTopic(Irccd &irccd, 
			     const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &topic)
	{
		util::unused(irccd, server, origin, channel, topic);
	}

	/**
	 * On unload.
	 *
	 * \param irccd the irccd instance
	 */
	virtual void onUnload(Irccd &irccd)
	{
		util::unused(irccd);
	}

	/**
	 * On whois information.
	 *
	 * \param irccd the irccd instance
	 * \param server the server
	 * \param info the info
	 */
	virtual void onWhois(Irccd &irccd, const std::shared_ptr<Server> &server, const ServerWhois &info)
	{
		util::unused(irccd, server, info);
	}
};

} // !irccd

#endif // !IRCCD_PLUGIN_HPP
