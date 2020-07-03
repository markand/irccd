/*
 * command_fixture.cpp -- test fixture helper for transport commands
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

#include <stdexcept>

#include <irccd/daemon/transport_command.hpp>
#include <irccd/daemon/transport_server.hpp>
#include <irccd/daemon/transport_service.hpp>

#include "command_fixture.hpp"

namespace irccd::test {

command_fixture::command_fixture()
	: server_(new mock_server(ctx_, "test", "localhost"))
	, plugin_(new mock_plugin("test"))
	, stream_(new mock_stream)
	, client_(new daemon::transport_client({}, stream_))
{
	// 1. Add all commands.
	for (const auto& f : daemon::transport_command::registry())
		bot_.get_transports().get_commands().push_back(f());

	bot_.get_servers().add(server_);
	bot_.get_plugins().add(plugin_);
	server_->disconnect();
	server_->clear();
	plugin_->clear();
}

auto command_fixture::request(nlohmann::json json) -> nlohmann::json
{
	const auto& list = bot_.get_transports().get_commands();
	const auto cmd = std::find_if(list.begin(), list.end(), [&] (const auto& c) {
		return c->get_name() == json["command"].template get<std::string>();
	});

	if (cmd == list.end())
		throw std::runtime_error("command not found");

	try {
		(*cmd)->exec(bot_, *client_, json);
	} catch (const std::system_error& ex) {
		client_->error(ex.code(), (*cmd)->get_name());
	}

	const auto& queue = stream_->find("send");

	if (queue.size() > 0)
		return nlohmann::json::parse(std::any_cast<std::string>(queue[0][0]));

	return nullptr;
}

} // !irccd::test
