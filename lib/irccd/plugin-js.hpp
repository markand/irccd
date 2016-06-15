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
 * \file plugin-js.hpp
 * \brief JavaScript plugins for irccd.
 */

#include "duktape.hpp"
#include "path.hpp"
#include "plugin.hpp"
#include "signals.hpp"

namespace irccd {

class Module;

/**
 * \brief JavaScript plugins for irccd.
 * \ingroup plugins
 */
class JsPlugin : public Plugin {
private:
	// JavaScript context
	UniqueContext m_context;

	// Store loaded modules.
	std::vector<std::shared_ptr<Module>> m_modules;

	// Private helpers.
	std::unordered_map<std::string, std::string> getTable(const char *name) const;
	void putTable(const char *name, const std::unordered_map<std::string, std::string> &vars);
	void call(const std::string &name, unsigned nargs = 0);
	void putModules(Irccd &irccd);
	void putVars();
	void putPath(const std::string &varname, const std::string &append, path::Path type);

public:
	/**
	 * Constructor.
	 *
	 * \param name the plugin name
	 * \param path the path to the plugin
	 */
	IRCCD_EXPORT JsPlugin(std::string name, std::string path);

	/**
	 * Access the Duktape context.
	 *
	 * \return the context
	 */
	inline UniqueContext &context() noexcept
	{
		return m_context;
	}

	IRCCD_EXPORT PluginConfig config() override;

	IRCCD_EXPORT void setConfig(PluginConfig) override;

	IRCCD_EXPORT PluginFormats formats() override;

	IRCCD_EXPORT void setFormats(PluginFormats formats) override;

	/**
	 * \copydoc Plugin::onCommand
	 */
	IRCCD_EXPORT void onCommand(Irccd &irccd,
				    const std::shared_ptr<Server> &server,
				    const std::string &origin,
				    const std::string &channel,
				    const std::string &message) override;

	/**
	 * \copydoc Plugin::onConnect
	 */
	IRCCD_EXPORT void onConnect(Irccd &irccd, const std::shared_ptr<Server> &server) override;

	/**
	 * \copydoc Plugin::onChannelMode
	 */
	IRCCD_EXPORT void onChannelMode(Irccd &irccd,
					const std::shared_ptr<Server> &server,
					const std::string &origin,
					const std::string &channel,
					const std::string &mode,
					const std::string &arg) override;

	/**
	 * \copydoc Plugin::onChannelNotice
	 */
	IRCCD_EXPORT void onChannelNotice(Irccd &irccd,
					  const std::shared_ptr<Server> &server,
					  const std::string &origin,
					  const std::string &channel,
					  const std::string &notice) override;

	/**
	 * \copydoc Plugin::onInvite
	 */
	IRCCD_EXPORT void onInvite(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel) override;

	/**
	 * \copydoc Plugin::onJoin
	 */
	IRCCD_EXPORT void onJoin(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel) override;

	/**
	 * \copydoc Plugin::onKick
	 */
	IRCCD_EXPORT void onKick(Irccd &irccd,
				 const std::shared_ptr<Server> &server,
				 const std::string &origin,
				 const std::string &channel,
				 const std::string &target,
				 const std::string &reason) override;

	/**
	 * \copydoc Plugin::onLoad
	 */
	IRCCD_EXPORT void onLoad(Irccd &irccd) override;

	/**
	 * \copydoc Plugin::onMessage
	 */
	IRCCD_EXPORT void onMessage(Irccd &irccd,
				    const std::shared_ptr<Server> &server,
				    const std::string &origin,
				    const std::string &channel,
				    const std::string &message) override;

	/**
	 * \copydoc Plugin::onMe
	 */
	IRCCD_EXPORT void onMe(Irccd &irccd,
			       const std::shared_ptr<Server> &server,
			       const std::string &origin,
			       const std::string &channel,
			       const std::string &message) override;

	/**
	 * \copydoc Plugin::onMode
	 */
	IRCCD_EXPORT void onMode(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode) override;

	/**
	 * \copydoc Plugin::onNames
	 */
	IRCCD_EXPORT void onNames(Irccd &irccd,
				  const std::shared_ptr<Server> &server,
				  const std::string &channel,
				  const std::vector<std::string> &list) override;

	/**
	 * \copydoc Plugin::onNick
	 */
	IRCCD_EXPORT void onNick(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick) override;

	/**
	 * \copydoc Plugin::onNotice
	 */
	IRCCD_EXPORT void onNotice(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice) override;

	/**
	 * \copydoc Plugin::onPart
	 */
	IRCCD_EXPORT void onPart(Irccd &irccd,
				 const std::shared_ptr<Server> &server,
				 const std::string &origin,
				 const std::string &channel,
				 const std::string &reason) override;

	/**
	 * \copydoc Plugin::onQuery
	 */
	IRCCD_EXPORT void onQuery(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message) override;

	/**
	 * \copydoc Plugin::onQueryCommand
	 */
	IRCCD_EXPORT void onQueryCommand(Irccd &irccd,
			    const std::shared_ptr<Server> &server,
			    const std::string &origin,
			    const std::string &message) override;

	/**
	 * \copydoc Plugin::onReload
	 */
	IRCCD_EXPORT void onReload(Irccd &irccd) override;

	/**
	 * \copydoc Plugin::onTopic
	 */
	IRCCD_EXPORT void onTopic(Irccd &irccd,
				  const std::shared_ptr<Server> &server,
				  const std::string &origin,
				  const std::string &channel,
				  const std::string &topic) override;

	/**
	 * \copydoc Plugin::onUnload
	 */
	IRCCD_EXPORT void onUnload(Irccd &irccd) override;

	/**
	 * \copydoc Plugin::onWhois
	 */
	IRCCD_EXPORT void onWhois(Irccd &irccd, const std::shared_ptr<Server> &server, const ServerWhois &info) override;
};

} // !irccd

#endif // !IRCCD_PLUGIN_JS_HPP
