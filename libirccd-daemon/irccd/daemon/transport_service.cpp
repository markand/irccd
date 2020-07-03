/*
 * transport_service.cpp -- transport service
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

#include <irccd/sysconfig.hpp>

#include <cassert>

#include <irccd/json_util.hpp>

#include "bot.hpp"
#include "logger.hpp"
#include "transport_client.hpp"
#include "transport_command.hpp"
#include "transport_server.hpp"
#include "transport_service.hpp"
#include "transport_util.hpp"

namespace irccd::daemon {

void transport_service::handle_command(std::shared_ptr<transport_client> client, const nlohmann::json& object)
{
	assert(object.is_object());

	const json_util::deserializer doc(object);
	const auto name = doc.get<std::string>("command");

	if (!name) {
		client->error(bot_error::invalid_message);
		return;
	}

	const auto cmd = std::find_if(commands_.begin(), commands_.end(), [&] (const auto& cptr) {
		return cptr->get_name() == *name;
	});

	if (cmd == commands_.end())
		client->error(bot_error::invalid_command, *name);
	else {
		try {
			(*cmd)->exec(bot_, *client, doc);
		} catch (const std::system_error& ex) {
			client->error(ex.code(), (*cmd)->get_name());
		} catch (const std::exception& ex) {
			bot_.get_log().warning("transport", "")
				<< "unknown error not reported: "
				<< ex.what() << std::endl;
		}
	}
}

void transport_service::recv(std::shared_ptr<transport_client> client)
{
	client->read([this, client] (auto code, auto json) {
		switch (static_cast<std::errc>(code.value())) {
		case std::errc::not_connected:
			bot_.get_log().info("transport", "") << "client disconnected" << std::endl;
			break;
		case std::errc::invalid_argument:
			client->error(bot_error::invalid_message);
			break;
		default:
			// Other errors.
			if (!code) {
				handle_command(client, json);

				if (client->get_state() == transport_client::state::ready)
					recv(std::move(client));
			}

			break;
		}
	});
}

void transport_service::handshake(std::shared_ptr<transport_client> client)
{
	client->handshake([this, client] (auto code) {
		if (code)
			bot_.get_log().warning("transport", "")
				<< "error while handshaking: " << code.message() << std::endl;
		else {
			bot_.get_log().info("transport", "") << "client ready" << std::endl;
			recv(std::move(client));
		}
	});
}

void transport_service::accept(transport_server& ts)
{
	ts.accept([this, &ts] (auto code, auto client) {
		if (!code) {
			accept(ts);
			handshake(std::move(client));

			bot_.get_log().info("transport", "") << "new client connected" << std::endl;
		}
	});
}

transport_service::transport_service(bot& bot) noexcept
	: bot_(bot)
{
}

transport_service::~transport_service() noexcept = default;

auto transport_service::get_commands() const noexcept -> const commands&
{
	return commands_;
}

auto transport_service::get_commands() noexcept -> commands&
{
	return commands_;
}

void transport_service::add(std::shared_ptr<transport_server> ts)
{
	assert(ts);

	accept(*ts);
	servers_.push_back(std::move(ts));
}

void transport_service::broadcast(const nlohmann::json& json)
{
	assert(json.is_object());

	for (const auto& servers : servers_)
		for (const auto& client : servers->get_clients())
			client->write(json);
}

void transport_service::load(const config& cfg) noexcept
{
	for (const auto& section : cfg) {
		if (section.get_key() != "transport")
			continue;

		try {
			add(transport_util::from_config(bot_.get_service(), section));
		} catch (const std::exception& ex) {
			bot_.get_log().warning("transport", "") << ex.what() << std::endl;
		}
	}
}

} // !irccd::daemon
