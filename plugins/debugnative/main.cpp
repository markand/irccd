/*
 * main.cpp -- native plugin for debugging
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

#include <iostream>

#include <irccd.hpp>
#include <logger.hpp>
#include <plugin-dynlib.hpp>
#include <server.hpp>
#include <util.hpp>

using namespace irccd;

extern "C" {

DYNLIB_EXPORT void irccd_onChannelMode(Irccd &, const ChannelModeEvent &event)
{
    log::info() << "plugin debugnative: onChannelMode event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   mode: " << event.mode << std::endl;
    log::info() << "plugin debugnative:   argument: " << event.argument << std::endl;
}

DYNLIB_EXPORT void irccd_onChannelNotice(Irccd &, const ChannelNoticeEvent &event)
{
    log::info() << "plugin debugnative: onChannelNotice event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onCommand(Irccd &, const MessageEvent &event)
{
    log::info() << "plugin debugnative: onCommand event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onConnect(Irccd &, const ConnectEvent &event)
{
    log::info() << "plugin debugnative: onConnect event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
}

DYNLIB_EXPORT void irccd_onInvite(Irccd &, const InviteEvent &event)
{
    log::info() << "plugin debugnative: onInvite event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   nickname: " << event.nickname << std::endl;
}

DYNLIB_EXPORT void irccd_onJoin(Irccd &, const JoinEvent &event)
{
    log::info() << "plugin debugnative: onJoin event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
}

DYNLIB_EXPORT void irccd_onKick(Irccd &, const KickEvent &event)
{
    log::info() << "plugin debugnative: onKick event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   target: " << event.target << std::endl;
    log::info() << "plugin debugnative:   reason: " << event.reason << std::endl;
}

DYNLIB_EXPORT void irccd_onLoad(Irccd &, DynlibPlugin &)
{
    log::info() << "plugin debugnative: onLoad event\n";
}

DYNLIB_EXPORT void irccd_onMessage(Irccd &, const MessageEvent &event)
{

    log::info() << "plugin debugnative: onMessage event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onMe(Irccd &, const MeEvent &event)
{
    log::info() << "plugin debugnative: onMe event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onMode(Irccd &, const ModeEvent &event)
{
    log::info() << "plugin debugnative: onMode event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   mode: " << event.mode << std::endl;
}

DYNLIB_EXPORT void irccd_onNames(Irccd &, const NamesEvent &event)
{
    log::info() << "plugin debugnative: onNames event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   names: "
                << util::join(event.names.begin(), event.names.end(), ", ")
                << std::endl;
}

DYNLIB_EXPORT void irccd_onNick(Irccd &, const NickEvent &event)
{
    log::info() << "plugin debugnative: onNick event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   nickname: " << event.nickname << std::endl;
}

DYNLIB_EXPORT void irccd_onNotice(Irccd &, const NoticeEvent &event)
{
    log::info() << "plugin debugnative: onNotice event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onPart(Irccd &, const PartEvent &event)
{
    log::info() << "plugin debugnative: onPart event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   reason: " << event.reason << std::endl;
}

DYNLIB_EXPORT void irccd_onQuery(Irccd &, const QueryEvent &event)
{
    log::info() << "plugin debugnative: onQuery event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onQueryCommand(Irccd &, const QueryEvent &event)
{
    log::info() << "plugin debugnative: onQueryCommand event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   message: " << event.message << std::endl;
}

DYNLIB_EXPORT void irccd_onReload(Irccd &, DynlibPlugin &)
{
    log::info() << "plugin debugnative: onReload event\n";
}

DYNLIB_EXPORT void irccd_onTopic(Irccd &, const TopicEvent &event)
{
    log::info() << "plugin debugnative: onTopic event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   origin: " << event.origin << std::endl;
    log::info() << "plugin debugnative:   channel: " << event.channel << std::endl;
    log::info() << "plugin debugnative:   topic: " << event.topic << std::endl;
}

DYNLIB_EXPORT void irccd_onUnload(Irccd &, DynlibPlugin &)
{
    log::info() << "plugin debugnative: onUnload event\n";
}

DYNLIB_EXPORT void irccd_onWhois(Irccd &, const WhoisEvent &event)
{
    log::info() << "plugin debugnative: onWhois event\n";
    log::info() << "plugin debugnative:   server: " << event.server->name() << std::endl;
    log::info() << "plugin debugnative:   nick: " << event.whois.nick << std::endl;
    log::info() << "plugin debugnative:   user: " << event.whois.user << std::endl;
    log::info() << "plugin debugnative:   host: " << event.whois.host << std::endl;
    log::info() << "plugin debugnative:   channels: "
                << util::join(event.whois.channels.begin(),
                              event.whois.channels.end(), ", ")
                << std::endl;
}

} // !C
