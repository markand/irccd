/*
 * hook.cpp -- irccd hooks
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#include <cassert>
#include <initializer_list>
#include <string_view>
#include <sstream>

#include <boost/process.hpp>

#include <irccd/string_util.hpp>

#include "bot.hpp"
#include "hook.hpp"
#include "logger.hpp"
#include "server.hpp"

using boost::process::args;
using boost::process::child;
using boost::process::exe;
using boost::process::ipstream;
using boost::process::std_out;

using std::getline;
using std::initializer_list;
using std::ostringstream;
using std::string;
using std::string_view;

using irccd::string_util::is_identifier;

namespace irccd::daemon {

namespace {

void exec(bot& bot, hook& hook, initializer_list<string> arguments)
{
	try {
		ipstream is;
		child c(exe = hook.get_path(), args = arguments, std_out > is);

		// Log everything that is output by the hook.
		for (string line; c.running() && getline(is, line); )
			bot.get_log().info(hook) << line << std::endl;

		c.wait();
	} catch (const std::exception& ex) {
		throw hook_error(hook_error::exec_error, hook.get_id(), ex.what());
	}
}

} // !namespace

hook::hook(std::string id, std::string path) noexcept
	: id_(std::move(id))
	, path_(std::move(path))
{
	assert(is_identifier(id_));
	assert(!path_.empty());
}

auto hook::get_id() const noexcept -> const std::string&
{
	return id_;
}

auto hook::get_path() const noexcept -> const std::string&
{
	return path_;
}

void hook::handle_connect(bot& bot, const connect_event& event)
{
	exec(bot, *this, {"onConnect", event.server->get_id()});
}

void hook::handle_disconnect(bot& bot, const disconnect_event& event)
{
	exec(bot, *this, {"onDisconnect", event.server->get_id()});
}

void hook::handle_invite(bot& bot, const invite_event& event)
{
	exec(bot, *this, {
		"onInvite",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.nickname
	});
}

void hook::handle_join(bot& bot, const join_event& event)
{
	exec(bot, *this, {
		"onJoin",
		event.server->get_id(),
		event.origin,
		event.channel
	});
}

void hook::handle_kick(bot& bot, const kick_event& event)
{
	exec(bot, *this, {
		"onKick",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.target,
		event.reason
	});
}

void hook::handle_message(bot& bot, const message_event& event)
{
	exec(bot, *this, {
		"onMessage",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.message
	});
}

void hook::handle_me(bot& bot, const me_event& event)
{
	exec(bot, *this, {
		"onMe",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.message
	});
}

void hook::handle_mode(bot& bot, const mode_event& event)
{
	exec(bot, *this, {
		"onMode",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.mode,
		event.limit,
		event.user,
		event.mask
	});
}

void hook::handle_nick(bot& bot, const nick_event& event)
{
	exec(bot, *this, {
		"onNick",
		event.server->get_id(),
		event.origin,
		event.nickname
	});
}

void hook::handle_notice(bot& bot, const notice_event& event)
{
	exec(bot, *this, {
		"onNotice",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.message
	});
}

void hook::handle_part(bot& bot, const part_event& event)
{
	exec(bot, *this, {
		"onPart",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.reason
	});
}

void hook::handle_topic(bot& bot, const topic_event& event)
{
	exec(bot, *this, {
		"onTopic",
		event.server->get_id(),
		event.origin,
		event.channel,
		event.topic
	});
}

auto operator==(const hook& h1, const hook& h2) noexcept -> bool
{
	return h1.get_id() == h2.get_id() &&
	       h1.get_path() == h2.get_path();
}

auto operator!=(const hook& h1, const hook& h2) noexcept -> bool
{
	return !(h1 == h2);
}

hook_error::hook_error(error errc, std::string id, std::string message)
	: system_error(make_error_code(errc))
	, id_(std::move(id))
	, message_(std::move(message))
{
}

auto hook_error::get_id() const noexcept -> const std::string&
{
	return id_;
}

auto hook_error::get_message() const noexcept -> const std::string&
{
	return message_;
}

auto hook_error::what() const noexcept -> const char*
{
	return message_.c_str();
}

auto hook_category() -> const std::error_category&
{
	static const class category : public std::error_category {
	public:
		auto name() const noexcept -> const char* override
		{
			return "hook";
		}

		auto message(int e) const -> std::string override
		{
			switch (static_cast<hook_error::error>(e)) {
			case hook_error::not_found:
				return "hook not found";
			case hook_error::invalid_path:
				return "invalid path given";
			case hook_error::invalid_identifier:
				return "invalid hook identifier";
			case hook_error::exec_error:
				return "hook exec error";
			case hook_error::already_exists:
				return "hook already exists";
			default:
				return "no error";
			}
		}
	} category;

	return category;
}

auto make_error_code(hook_error::error e) -> std::error_code
{
	return { static_cast<int>(e), hook_category() };
}

namespace logger {

auto type_traits<hook>::get_category(const hook&) -> std::string_view
{
	return "hook";
}

auto type_traits<hook>::get_component(const hook& hook) -> std::string_view
{
	return hook.get_id();
}

} // !logger

} // !irccd::daemon
