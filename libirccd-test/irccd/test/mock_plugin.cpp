/*
 * mock_plugin.cpp -- mock plugin
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

#include "mock_plugin.hpp"

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

auto mock_plugin::get_name() const noexcept -> std::string_view
{
	push("get_name");

	return "mock";
}

auto mock_plugin::get_author() const noexcept -> std::string_view
{
	push("get_author");

	return "David Demelier <markand@malikania.fr>";
}

auto mock_plugin::get_license() const noexcept -> std::string_view
{
	push("get_license");

	return "ISC";
}

auto mock_plugin::get_summary() const noexcept -> std::string_view
{
	push("get_summary");

	return "mock plugin";
}

auto mock_plugin::get_version() const noexcept -> std::string_view
{
	push("get_version");

	return "1.0";
}

auto mock_plugin::get_options() const -> map
{
	push("get_options");

	return options_;
}

void mock_plugin::set_options(const map& map)
{
	push("set_options", { map });

	options_ = map;
}

auto mock_plugin::get_templates() const -> map
{
	push("get_templates");

	return templates_;
}

void mock_plugin::set_templates(const map& map)
{
	push("set_templates", { map });

	templates_ = map;
}

auto mock_plugin::get_paths() const -> map
{
	push("get_paths");

	return paths_;
}

void mock_plugin::set_paths(const map& map)
{
	push("set_paths", { map });

	paths_ = map;
}

void mock_plugin::handle_command(bot&, const message_event& event)
{
	push("handle_command", { event });
}

void mock_plugin::handle_connect(bot&, const connect_event& event)
{
	push("handle_connect", { event });
}

void mock_plugin::handle_disconnect(bot&, const disconnect_event& event)
{
	push("handle_disconnect", { event });
}

void mock_plugin::handle_invite(bot&, const invite_event& event)
{
	push("handle_invite", { event });
}

void mock_plugin::handle_join(bot&, const join_event& event)
{
	push("handle_join", { event });
}

void mock_plugin::handle_kick(bot&, const kick_event& event)
{
	push("handle_kick", { event });
}

void mock_plugin::handle_load(bot&)
{
	push("handle_load");
}

void mock_plugin::handle_message(bot&, const message_event& event)
{
	push("handle_message", { event });
}

void mock_plugin::handle_me(bot&, const me_event& event)
{
	push("handle_me", { event });
}

void mock_plugin::handle_mode(bot&, const mode_event& event)
{
	push("handle_mode", { event });
}

void mock_plugin::handle_names(bot&, const names_event& event)
{
	push("handle_names", { event });
}

void mock_plugin::handle_nick(bot&, const nick_event& event)
{
	push("handle_nick", { event });
}

void mock_plugin::handle_notice(bot&, const notice_event& event)
{
	push("handle_notice", { event });
}

void mock_plugin::handle_part(bot&, const part_event& event)
{
	push("handle_part", { event });
}

void mock_plugin::handle_reload(bot&)
{
	push("handle_reload");
}

void mock_plugin::handle_topic(bot&, const topic_event& event)
{
	push("handle_topic", { event });
}

void mock_plugin::handle_unload(bot&)
{
	push("handle_unload");
}

void mock_plugin::handle_whois(bot&, const whois_event& event)
{
	push("handle_whois", { event });
}

} // !irccd::test
