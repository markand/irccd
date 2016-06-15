/*
 * plugin-dynlib.cpp -- native plugin implementation
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

#include "plugin-dynlib.hpp"

namespace irccd {

namespace {

template <typename Sym>
inline Sym sym(Dynlib &dynlib, const std::string &name)
{
	try {
		return dynlib.sym<Sym>(name);
	} catch (...) {
		return nullptr;
	}
}

template <typename Sym, typename... Args>
inline void call(Sym sym, Args&&... args)
{
	if (sym)
		sym(std::forward<Args>(args)...);
}

} // !namespace

DynlibPlugin::DynlibPlugin(std::string name, std::string path)
	: Plugin(name, path)
	, m_dso(std::move(path))
	, m_onCommand(sym<OnCommand>(m_dso, "irccd_onCommand"))
	, m_onConnect(sym<OnConnect>(m_dso, "irccd_onConnect"))
	, m_onChannelMode(sym<OnChannelMode>(m_dso, "irccd_onChannelMode"))
	, m_onChannelNotice(sym<OnChannelNotice>(m_dso, "irccd_onChannelNotice"))
	, m_onInvite(sym<OnInvite>(m_dso, "irccd_onInvite"))
	, m_onJoin(sym<OnJoin>(m_dso, "irccd_onJoin"))
	, m_onKick(sym<OnKick>(m_dso, "irccd_onKick"))
	, m_onLoad(sym<OnLoad>(m_dso, "irccd_onLoad"))
	, m_onMessage(sym<OnMessage>(m_dso, "irccd_onMessage"))
	, m_onMe(sym<OnMe>(m_dso, "irccd_onMe"))
	, m_onMode(sym<OnMode>(m_dso, "irccd_onMode"))
	, m_onNames(sym<OnNames>(m_dso, "irccd_onNames"))
	, m_onNick(sym<OnNick>(m_dso, "irccd_onNick"))
	, m_onNotice(sym<OnNotice>(m_dso, "irccd_onNotice"))
	, m_onPart(sym<OnPart>(m_dso, "irccd_onPart"))
	, m_onQuery(sym<OnQuery>(m_dso, "irccd_onQuery"))
	, m_onQueryCommand(sym<OnQueryCommand>(m_dso, "irccd_onQueryCommand"))
	, m_onReload(sym<OnReload>(m_dso, "irccd_onReload"))
	, m_onTopic(sym<OnTopic>(m_dso, "irccd_onTopic"))
	, m_onUnload(sym<OnUnload>(m_dso, "irccd_onUnload"))
	, m_onWhois(sym<OnWhois>(m_dso, "irccd_onWhois"))
{
}

void DynlibPlugin::onCommand(Irccd &irccd,
			     const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &message)
{
	call(m_onCommand, irccd, server, origin, channel, message);
}

void DynlibPlugin::onConnect(Irccd &irccd, const std::shared_ptr<Server> &server)
{
	call(m_onConnect, irccd, server);
}

void DynlibPlugin::onChannelMode(Irccd &irccd,
				 const std::shared_ptr<Server> &server,
				 const std::string &origin,
				 const std::string &channel,
				 const std::string &mode,
				 const std::string &arg)
{
	call(m_onChannelMode, irccd, server, origin, channel, mode, arg);
}

void DynlibPlugin::onChannelNotice(Irccd &irccd,
				   const std::shared_ptr<Server> &server,
				   const std::string &origin,
				   const std::string &channel,
				   const std::string &notice)
{
	call(m_onChannelNotice, irccd, server, origin, channel, notice);
}

void DynlibPlugin::onInvite(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	call(m_onInvite, irccd, server, origin, channel);
}

void DynlibPlugin::onJoin(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &channel)
{
	call(m_onJoin, irccd, server, origin, channel);
}

void DynlibPlugin::onKick(Irccd &irccd,
			  const std::shared_ptr<Server> &server,
			  const std::string &origin,
			  const std::string &channel,
			  const std::string &target,
			  const std::string &reason)
{
	call(m_onKick, irccd, server, origin, channel, target, reason);
}

void DynlibPlugin::onLoad(Irccd &irccd)
{
	call(m_onLoad, irccd, *this);
}

void DynlibPlugin::onMessage(Irccd &irccd,
			     const std::shared_ptr<Server> &server,
			     const std::string &origin,
			     const std::string &channel,
			     const std::string &message)
{
	call(m_onMessage, irccd, server, origin, channel, message);
}

void DynlibPlugin::onMe(Irccd &irccd,
			const std::shared_ptr<Server> &server,
			const std::string &origin,
			const std::string &channel,
			const std::string &message)
{
	call(m_onMe, irccd, server, origin, channel, message);
}

void DynlibPlugin::onMode(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &mode)
{
	call(m_onMode, irccd, server, origin, mode);
}

void DynlibPlugin::onNames(Irccd &irccd,
			   const std::shared_ptr<Server> &server,
			   const std::string &channel,
			   const std::vector<std::string> &list)
{
	call(m_onNames, irccd, server, channel, list);
}

void DynlibPlugin::onNick(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &nick)
{
	call(m_onNick, irccd, server, origin, nick);
}

void DynlibPlugin::onNotice(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &notice)
{
	call(m_onNotice, irccd, server, origin, notice);
}

void DynlibPlugin::onPart(Irccd &irccd,
			  const std::shared_ptr<Server> &server,
			  const std::string &origin,
			  const std::string &channel,
			  const std::string &reason)
{
	call(m_onPart, irccd, server, origin, channel, reason);
}

void DynlibPlugin::onQuery(Irccd &irccd, const std::shared_ptr<Server> &server, const std::string &origin, const std::string &message)
{
	call(m_onQuery, irccd, server, origin, message);
}

void DynlibPlugin::onQueryCommand(Irccd &irccd,
				  const std::shared_ptr<Server> &server,
				  const std::string &origin,
				  const std::string &message)
{
	call(m_onQueryCommand, irccd, server, origin, message);
}

void DynlibPlugin::onReload(Irccd &irccd)
{
	call(m_onReload, irccd, *this);
}

void DynlibPlugin::onTopic(Irccd &irccd,
			   const std::shared_ptr<Server> &server,
			   const std::string &origin,
			   const std::string &channel,
			   const std::string &topic)
{
	call(m_onTopic, irccd, server, origin, channel, topic);
}

void DynlibPlugin::onUnload(Irccd &irccd)
{
	call(m_onUnload, irccd, *this);
}

void DynlibPlugin::onWhois(Irccd &irccd, const std::shared_ptr<Server> &server, const ServerWhois &info)
{
	call(m_onWhois, irccd, server, info);
}

} // !irccd
