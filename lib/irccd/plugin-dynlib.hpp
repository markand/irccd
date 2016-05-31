/*
 * plugin-dynlib.hpp -- native plugin implementation
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

#ifndef IRCCD_PLUGIN_DYNLIB_HPP
#define IRCCD_PLUGIN_DYNLIB_HPP

/**
 * \file plugin-dynlib.hpp
 * \brief Native plugin implementation.
 */

#include "dynlib.hpp"
#include "plugin.hpp"

namespace irccd {

/**
 * \brief Dynlib based plugin.
 * \ingroup plugins
 */
class DynlibPlugin : public Plugin {
private:
	using OnCommand = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &);
	using OnConnect = void (*)(Irccd &, const std::shared_ptr<Server> &);
	using OnChannelMode = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &, const std::string &);
	using OnChannelNotice = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &);
	using OnInvite = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnJoin = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnKick = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &, const std::string &);
	using OnLoad = void (*)(Irccd &, DynlibPlugin &);
	using OnMessage = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &);
	using OnMe = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &);
	using OnMode = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnNames = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::vector<std::string> &);
	using OnNick = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnNotice = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnPart = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &);
	using OnQuery = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnQueryCommand = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &);
	using OnReload = void (*)(Irccd &, DynlibPlugin &);
	using OnTopic = void (*)(Irccd &, const std::shared_ptr<Server> &, const std::string &, const std::string &, const std::string &);
	using OnUnload = void (*)(Irccd &, DynlibPlugin &);
	using OnWhois = void (*)(Irccd &, const std::shared_ptr<Server> &, const ServerWhois &);

	Dynlib m_dso;
	OnCommand m_onCommand;
	OnConnect m_onConnect;
	OnChannelMode m_onChannelMode;
	OnChannelNotice m_onChannelNotice;
	OnInvite m_onInvite;
	OnJoin m_onJoin;
	OnKick m_onKick;
	OnLoad m_onLoad;
	OnMessage m_onMessage;
	OnMe m_onMe;
	OnMode m_onMode;
	OnNames m_onNames;
	OnNick m_onNick;
	OnNotice m_onNotice;
	OnPart m_onPart;
	OnQuery m_onQuery;
	OnQueryCommand m_onQueryCommand;
	OnReload m_onReload;
	OnTopic m_onTopic;
	OnUnload m_onUnload;
	OnWhois m_onWhois;

public:
	/**
	 * Construct the plugin.
	 *
	 * \param name the name
	 * \param path the fully resolved path (must be absolute)
	 * \param config the optional configuration
	 * \throw std::exception on failures
	 */
	DynlibPlugin(std::string name, std::string path, PluginConfig config = PluginConfig());

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

#endif // !IRCCD_PLUGIN_DYNLIB_HPP
