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

void DynlibPlugin::onCommand(Irccd &irccd, const MessageEvent &ev)
{
    call(m_onCommand, irccd, ev);
}

void DynlibPlugin::onConnect(Irccd &irccd, const ConnectEvent &ev)
{
    call(m_onConnect, irccd, ev);
}

void DynlibPlugin::onChannelMode(Irccd &irccd, const ChannelModeEvent &ev)
{
    call(m_onChannelMode, irccd, ev);
}

void DynlibPlugin::onChannelNotice(Irccd &irccd, const ChannelNoticeEvent &ev)
{
    call(m_onChannelNotice, irccd, ev);
}

void DynlibPlugin::onInvite(Irccd &irccd, const InviteEvent &ev)
{
    call(m_onInvite, irccd, ev);
}

void DynlibPlugin::onJoin(Irccd &irccd, const JoinEvent &ev)
{
    call(m_onJoin, irccd, ev);
}

void DynlibPlugin::onKick(Irccd &irccd, const KickEvent &ev)
{
    call(m_onKick, irccd, ev);
}

void DynlibPlugin::onLoad(Irccd &irccd)
{
    call(m_onLoad, irccd, *this);
}

void DynlibPlugin::onMessage(Irccd &irccd, const MessageEvent &ev)
{
    call(m_onMessage, irccd, ev);
}

void DynlibPlugin::onMe(Irccd &irccd, const MeEvent &ev)
{
    call(m_onMe, irccd, ev);
}

void DynlibPlugin::onMode(Irccd &irccd, const ModeEvent &ev)
{
    call(m_onMode, irccd, ev);
}

void DynlibPlugin::onNames(Irccd &irccd, const NamesEvent &ev)
{
    call(m_onNames, irccd, ev);
}

void DynlibPlugin::onNick(Irccd &irccd, const NickEvent &ev)
{
    call(m_onNick, irccd, ev);
}

void DynlibPlugin::onNotice(Irccd &irccd, const NoticeEvent &ev)
{
    call(m_onNotice, irccd, ev);
}

void DynlibPlugin::onPart(Irccd &irccd, const PartEvent &ev)
{
    call(m_onPart, irccd, ev);
}

void DynlibPlugin::onQuery(Irccd &irccd, const QueryEvent &ev)
{
    call(m_onQuery, irccd, ev);
}

void DynlibPlugin::onQueryCommand(Irccd &irccd, const QueryEvent &ev)
{
    call(m_onQueryCommand, irccd, ev);
}

void DynlibPlugin::onReload(Irccd &irccd)
{
    call(m_onReload, irccd, *this);
}

void DynlibPlugin::onTopic(Irccd &irccd, const TopicEvent &ev)
{
    call(m_onTopic, irccd, ev);
}

void DynlibPlugin::onUnload(Irccd &irccd)
{
    call(m_onUnload, irccd, *this);
}

void DynlibPlugin::onWhois(Irccd &irccd, const WhoisEvent &ev)
{
    call(m_onWhois, irccd, ev);
}

} // !irccd
