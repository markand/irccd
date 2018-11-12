/*
 * command_fixture.cpp -- test fixture helper for transport commands
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

#include <irccd/acceptor.hpp>
#include <irccd/connector.hpp>

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/transport_server.hpp>
#include <irccd/daemon/transport_service.hpp>

#include "command_fixture.hpp"

using boost::asio::ip::tcp;

namespace irccd::test {

template <typename Condition>
void command_fixture::wait_for(Condition&& cond)
{
	ctx_.reset();

	while (!cond())
		ctx_.poll();
}

auto command_fixture::request(nlohmann::json json) -> result
{
	result r;

	ctl_->send(std::move(json), [] (auto code) {
		if (code)
			throw std::system_error(std::move(code));
	});
	ctl_->recv([&] (auto code, auto message) {
		r.first = message;
		r.second = code;
	});
	wait_for([&] {
		return r.second || r.first.is_object();
	});

	return r;
}

command_fixture::command_fixture()
	: server_(new mock_server(ctx_, "test", "localhost"))
	, plugin_(new mock_plugin("test"))
{
	tcp::endpoint ep(tcp::v4(), 0U);
	tcp::acceptor raw_acceptor(irccd_.get_service(), std::move(ep));

	auto service = std::to_string(raw_acceptor.local_endpoint().port());
	auto acceptor = std::make_unique<ip_acceptor>(irccd_.get_service(), std::move(raw_acceptor));
	auto connector = std::make_unique<ip_connector>(irccd_.get_service(), "127.0.0.1", service, true, false);

	// 1. Add all commands.
	for (const auto& f : command::registry)
		irccd_.transports().get_commands().push_back(f());

	// 2. Create controller and transport server.
	ctl_ = std::make_unique<ctl::controller>(std::move(connector));
	irccd_.transports().add(std::make_unique<transport_server>(std::move(acceptor)));

	// 3. Wait for controller to connect.
	boost::asio::deadline_timer timer(ctx_);

	timer.expires_from_now(boost::posix_time::seconds(10));
	timer.async_wait([] (auto code) {
		if (code && code != boost::asio::error::operation_aborted)
			throw std::system_error(make_error_code(std::errc::timed_out));
	});

	bool connected = false;

	ctl_->connect([&] (auto code, auto) {
		timer.cancel();

		if (code)
			throw std::system_error(code);

		connected = true;
	});

	/**
	 * Irccd will block indefinitely since transport_service will wait for any
	 * new client again, so we need to check with a boolean.
	 */
	while (!connected)
		ctx_.poll();

	irccd_.servers().add(server_);
	irccd_.plugins().add(plugin_);
	server_->disconnect();
	server_->clear();
	plugin_->clear();
}

} // !irccd::test
