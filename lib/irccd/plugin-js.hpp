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

#include <unordered_set>

#include "js.hpp"
#include "path.hpp"
#include "plugin.hpp"
#include "signals.hpp"

namespace irccd {

class Module;
class Timer;

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
	 * \copydoc Plugin::onCommand
	 */
	void onCommand(Irccd &irccd,
		       const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &channel,
		       const std::string &message) override;

	/**
	 * \copydoc Plugin::onConnect
	 */
	void onConnect(Irccd &irccd, const std::shared_ptr<Server> &server) override;

	/**
	 * \copydoc Plugin::onChannelMode
	 */
	void onChannelMode(Irccd &irccd,
			   const std::shared_ptr<Server> &server,
			   const std::string &origin,
			   const std::string &channel,
			   const std::string &mode,
			   const std::string &arg) override;

	/**
	 * \copydoc Plugin::onChannelNotice
	 */
	void onChannelNotice(Irccd &irccd,
			     const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &notice) override;

	/**
	 * \copydoc Plugin::onInvite
	 */
	void onInvite(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel) override;

	/**
	 * \copydoc Plugin::onJoin
	 */
	void onJoin(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel) override;

	/**
	 * \copydoc Plugin::onKick
	 */
	void onKick(Irccd &irccd,
		    const std::shared_ptr<Server> &server,
		    const std::string &origin,
		    const std::string &channel,
		    const std::string &target,
		    const std::string &reason) override;

	/**
	 * \copydoc Plugin::onLoad
	 */
	void onLoad(Irccd &irccd) override;

	/**
	 * \copydoc Plugin::onMessage
	 */
	void onMessage(Irccd &irccd,
		       const std::shared_ptr<Server> &server,
		       const std::string &origin,
		       const std::string &channel,
		       const std::string &message) override;

	/**
	 * \copydoc Plugin::onMe
	 */
	void onMe(Irccd &irccd,
		  const std::shared_ptr<Server> &server,
		  const std::string &origin,
		  const std::string &channel,
		  const std::string &message) override;

	/**
	 * \copydoc Plugin::onMode
	 */
	void onMode(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode) override;

	/**
	 * \copydoc Plugin::onNames
	 */
	void onNames(Irccd &irccd,
		     const std::shared_ptr<Server> &server,
		     const std::string &channel,
		     const std::vector<std::string> &list) override;

	/**
	 * \copydoc Plugin::onNick
	 */
	void onNick(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick) override;

	/**
	 * \copydoc Plugin::onNotice
	 */
	void onNotice(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice) override;

	/**
	 * \copydoc Plugin::onPart
	 */
	void onPart(Irccd &irccd,
		    const std::shared_ptr<Server> &server,
		    const std::string &origin,
		    const std::string &channel,
		    const std::string &reason) override;

	/**
	 * \copydoc Plugin::onQuery
	 */
	void onQuery(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message) override;

	/**
	 * \copydoc Plugin::onQueryCommand
	 */
	void onQueryCommand(Irccd &irccd,
			    const std::shared_ptr<Server> &server,
			    const std::string &origin,
			    const std::string &message) override;

	/**
	 * \copydoc Plugin::onReload
	 */
	void onReload(Irccd &irccd) override;

	/**
	 * \copydoc Plugin::onTopic
	 */
	void onTopic(Irccd &irccd,
		     const std::shared_ptr<Server> &server,
		     const std::string &origin,
		     const std::string &channel,
		     const std::string &topic) override;

	/**
	 * \copydoc Plugin::onUnload
	 */
	void onUnload(Irccd &irccd) override;

	/**
	 * \copydoc Plugin::onWhois
	 */
	void onWhois(Irccd &irccd, const std::shared_ptr<Server> &server, const ServerWhois &info) override;
};

} // !irccd

#endif // !IRCCD_PLUGIN_JS_HPP
