/*
 * controller.cpp -- main irccdctl interface
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <irccd/sysconfig.hpp>
#include <irccd/json_util.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/server.hpp>
#include <irccd/daemon/plugin.hpp>
#include <irccd/daemon/rule.hpp>

#include "controller.hpp"

using irccd::json_util::deserializer;

using irccd::daemon::bot_error;
using irccd::daemon::plugin_error;
using irccd::daemon::rule_error;
using irccd::daemon::server_error;

namespace irccd::ctl {

void controller::authenticate(connect_handler handler, nlohmann::json info)
{
	const auto cmd = nlohmann::json::object({
		{ "command",    "auth"          },
		{ "password",   password_       }
	});

	send(cmd, [handler, info, this] (auto code) {
		if (code) {
			handler(std::move(code), nullptr);
			return;
		}

		recv([handler, info] (auto code, auto) {
			handler(std::move(code), std::move(info));
		});
	});
}

void controller::verify(connect_handler handler)
{
	recv([handler, this] (auto code, auto message) {
		if (code) {
			handler(std::move(code), std::move(message));
			return;
		}

		const deserializer doc(message);
		const auto program = doc.get<std::string>("program");
		const auto major = doc.get<int>("major");

		if (!program && *program != "irccd")
			handler(bot_error::not_irccd, std::move(message));
		else if (major && *major != IRCCD_VERSION_MAJOR)
			handler(bot_error::incompatible_version, std::move(message));
		else {
			if (!password_.empty())
				authenticate(std::move(handler), message);
			else
				handler(code, std::move(message));
		}
	});
}

controller::controller(std::unique_ptr<connector> connector) noexcept
	: connector_(std::move(connector))
{
	assert(connector_);
}

auto controller::get_password() const noexcept -> const std::string&
{
	return password_;
}

void controller::set_password(std::string password) noexcept
{
	password_ = std::move(password);
}

void controller::connect(connect_handler handler)
{
	assert(handler);

	connector_->connect([handler, this] (auto code, auto stream) {
		if (code)
			handler(std::move(code), nullptr);
		else {
			stream_ = std::move(stream);
			verify(std::move(handler));
		}
	});
}

void controller::recv(stream::recv_handler handler)
{
	assert(stream_);
	assert(handler);

	auto stream = stream_;

	stream_->recv([this, handler, stream] (auto code, auto msg) {
		if (code) {
			stream_ = nullptr;
			handler(std::move(code), std::move(msg));
			return;
		}

		const deserializer doc(msg);
		const auto e = doc.get<int>("error");
		const auto c = doc.get<std::string>("errorCategory");

		if (e && c) {
			if (*c == "irccd")
				code = make_error_code(static_cast<bot_error::error>(*e));
			else if (*c == "server")
				code = make_error_code(static_cast<server_error::error>(*e));
			else if (*c == "plugin")
				code = make_error_code(static_cast<plugin_error::error>(*e));
			else if (*c == "rule")
				code = make_error_code(static_cast<rule_error::error>(*e));
		}

		handler(std::move(code), std::move(msg));
	});
}

void controller::send(nlohmann::json message, stream::send_handler handler)
{
	assert(stream_);
	assert(message.is_object());
	assert(handler);

	auto stream = stream_;

	stream_->send(std::move(message), [this, stream, handler] (auto code) {
		if (code)
			stream_ = nullptr;
		if (handler)
			handler(std::move(code));
	});
}

} // !irccd::ctl
