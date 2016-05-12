/*
 * plugin-js.hpp -- JavaScript plugins for irccd
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

#ifndef IRCCD_PLUGIN_JS_HPP
#define IRCCD_PLUGIN_JS_HPP

/**
 * \file plugin-hs.hpp
 * \brief JavaScript plugins for irccd.
 */

#include "plugin.hpp"

namespace irccd {

/**
 * \brief Timers that a plugin owns.
 */
using PluginTimers = std::unordered_set<std::shared_ptr<Timer>>;

/**
 * \brief JavaScript plugins for irccd.
 */
class JsPlugin : public Plugin {
public:
	// TODO: remove with future modules

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
	 * Constructor.
	 *
	 * \param name the plugin name
	 * \param path the path to the plugin
	 * \param config the configuration
	 */
	JsPlugin(std::string name, std::string path, const PluginConfig &config = PluginConfig());

	/**
	 * Close timers.
	 */
	~JsPlugin();

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
	void onCommand(const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &channel,
		       const std::string &message) override;

	/**
	 * On successful connection.
	 *
	 * \param server the server
	 */
	void onConnect(const std::shared_ptr<Server> &server) override;

	/**
	 * On channel mode.
	 *
	 * \param server the server
	 * \param origin the ouser who has changed the mode
	 * \param channel the channel
	 * \param mode the mode
	 * \param arg the optional mode argument
	 */
	void onChannelMode(const std::shared_ptr<Server> &server,
			   const std::string &origin,
			   const std::string &channel,
			   const std::string &mode,
			   const std::string &arg) override;

	/**
	 * On a channel notice.
	 *
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param channel on which channel
	 * \param notice the message
	 */
	void onChannelNotice(const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &notice) override;

	/**
	 * On invitation.
	 *
	 * \param server the server
	 * \param origin the user who invited you
	 * \param channel the channel
	 */
	void onInvite(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel) override;

	/**
	 * On join.
	 *
	 * \param server the server
	 * \param origin the user who joined
	 * \param channel the channel
	 */
	void onJoin(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel) override;

	/**
	 * On kick.
	 *
	 * \param server the server
	 * \param origin the user who kicked the target
	 * \param channel the channel
	 * \param target the kicked target
	 * \param reason the optional reason
	 */
	void onKick(const std::shared_ptr<Server> &server,
		    const std::string &origin,
		    const std::string &channel,
		    const std::string &target,
		    const std::string &reason) override;

	/**
	 * On load.
	 */
	void onLoad() override;

	/**
	 * On channel message.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel
	 * \param message the message or command
	 */
	void onMessage(const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &message) override;

	/**
	 * On CTCP Action.
	 *
	 * \param server the server
	 * \param origin the user who sent the message
	 * \param channel the channel (may also be your nickname)
	 * \param message the message
	 */
	void onMe(const std::shared_ptr<Server> &server,
		  const std::string &origin,
		  const std::string &channel,
		  const std::string &message) override;

	/**
	 * On user mode change.
	 *
	 * \param server the server
	 * \param origin the person who changed the mode
	 * \param mode the new mode
	 */
	void onMode(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode) override;

	/**
	 * On names listing.
	 *
	 * \param server the server
	 * \param channel the channel
	 * \param list the list of nicknames
	 */
	void onNames(const std::shared_ptr<Server> &server, const std::string &channel, const std::vector<std::string> &list) override;

	/**
	 * On nick change.
	 *
	 * \param server the server
	 * \param origin the user that changed its nickname
	 * \param nick the new nickname
	 */
	void onNick(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick) override;

	/**
	 * On user notice.
	 *
	 * \param server the server
	 * \param origin the user who sent the notice
	 * \param notice the notice
	 */
	void onNotice(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice) override;

	/**
	 * On part.
	 *
	 * \param server the server
	 * \param origin the user who left
	 * \param channel the channel
	 * \param reason the optional reason
	 */
	void onPart(const std::shared_ptr<Server> &server,
		    const std::string &origin,
		    const std::string &channel,
		    const std::string &reason) override;

	/**
	 * On user query.
	 *
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	void onQuery(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message) override;

	/**
	 * On user query command.
	 *
	 * \param server the server
	 * \param origin the user who sent the query
	 * \param message the message
	 */
	void onQueryCommand(const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message) override;

	/**
	 * On reload.
	 */
	void onReload() override;

	/**
	 * On topic change.
	 *
	 * \param server the server
	 * \param origin the user who sent the topic
	 * \param channel the channel
	 * \param topic the new topic
	 */
	void onTopic(const std::shared_ptr<Server> &server,
		     const std::string &origin,
		     const std::string &channel,
		     const std::string &topic) override;

	/**
	 * On unload.
	 */
	void onUnload() override;

	/**
	 * On whois information.
	 *
	 * \param server the server
	 * \param info the info
	 */
	void onWhois(const std::shared_ptr<Server> &server, const ServerWhois &info) override;
};

} // !irccd

#endif // !IRCCD_PLUGIN_JS_HPP
