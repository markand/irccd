/*
 * broken_plugin.cpp -- broken plugin
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/server.hpp>

#include "broken_plugin.hpp"

using irccd::daemon::connect_event;
using irccd::daemon::disconnect_event;
using irccd::daemon::invite_event;
using irccd::daemon::bot;
using irccd::daemon::join_event;
using irccd::daemon::kick_event;
using irccd::daemon::me_event;
using irccd::daemon::message_event;
using irccd::daemon::mode_event;
using irccd::daemon::names_event;
using irccd::daemon::nick_event;
using irccd::daemon::notice_event;
using irccd::daemon::part_event;
using irccd::daemon::plugin;
using irccd::daemon::topic_event;
using irccd::daemon::whois_event;

namespace irccd::test {

auto broken_plugin::get_name() const noexcept -> std::string_view
{
	return "broken";
}

auto broken_plugin::get_author() const noexcept -> std::string_view
{
	return "David Demelier <markand@malikania.fr>";
}

auto broken_plugin::get_license() const noexcept -> std::string_view
{
	return "ISC";
}

auto broken_plugin::get_summary() const noexcept -> std::string_view
{
	return "broken plugin";
}

auto broken_plugin::get_version() const noexcept -> std::string_view
{
	return "1.0";
}

auto broken_plugin::get_options() const -> map
{
	throw std::runtime_error("broken");
}

void broken_plugin::set_options(const map&)
{
	throw std::runtime_error("broken");
}

auto broken_plugin::get_templates() const -> map
{
	throw std::runtime_error("broken");
}

void broken_plugin::set_templates(const map&)
{
	throw std::runtime_error("broken");
}

auto broken_plugin::get_paths() const -> map
{
	throw std::runtime_error("broken");
}

void broken_plugin::set_paths(const map&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_command(bot&, const message_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_connect(bot&, const connect_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_disconnect(bot&, const disconnect_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_invite(bot&, const invite_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_join(bot&, const join_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_kick(bot&, const kick_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_load(bot&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_message(bot&, const message_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_me(bot&, const me_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_mode(bot&, const mode_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_names(bot&, const names_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_nick(bot&, const nick_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_notice(bot&, const notice_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_part(bot&, const part_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_reload(bot&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_topic(bot&, const topic_event&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_unload(bot&)
{
	throw std::runtime_error("broken");
}

void broken_plugin::handle_whois(bot&, const whois_event&)
{
	throw std::runtime_error("broken");
}

} // !irccd::test
