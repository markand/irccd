/*
 * command_fixture.cpp -- test fixture helper for transport commands
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

#include <irccd/acceptor.hpp>
#include <irccd/connector.hpp>

#include <irccd/daemon/transport_command.hpp>
#include <irccd/daemon/transport_server.hpp>
#include <irccd/daemon/transport_service.hpp>

#include "command_fixture.hpp"

namespace asio = boost::asio;
namespace posix_time = boost::posix_time;

namespace irccd::test {

auto command_fixture::recv(asio::deadline_timer& timer) -> result
{
	result r;

	ctl_->recv([&] (auto code, auto message) {
		r.first = message;
		r.second = code;
	});

	while (!r.first.is_object() && !r.second) {
		ctx_.poll();
		ctx_.reset();
	}

	timer.cancel();

	return r;
}

auto command_fixture::wait_command(const std::string& cmd) -> result
{
	result r;
	asio::deadline_timer timer(bot_.get_service());

	timer.expires_from_now(posix_time::seconds(30));
	timer.async_wait([] (auto code) {
		if (code != asio::error::operation_aborted)
			throw std::runtime_error("operation timed out");
	});

	for (;;) {
		r = recv(timer);

		if (r.second)
			break;
		if (r.first.is_object() &&
		    r.first["command"].is_string() &&
		    r.first["command"].get<std::string>() == cmd)
			break;

		ctx_.poll();
		ctx_.reset();
	}

	return r;
}

auto command_fixture::request(nlohmann::json json) -> result
{
	ctl_->send(json, [] (auto code) {
		if (code)
			throw std::system_error(std::move(code));
	});

	return wait_command(json["command"].get<std::string>());
}

command_fixture::command_fixture()
	: server_(new mock_server(ctx_, "test", "localhost"))
	, plugin_(new mock_plugin("test"))
{
	asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 0U);
	asio::ip::tcp::acceptor raw_acceptor(bot_.get_service(), std::move(ep));

	auto service = std::to_string(raw_acceptor.local_endpoint().port());
	auto acceptor = std::make_unique<ip_acceptor>(bot_.get_service(), std::move(raw_acceptor));
	auto connector = std::make_unique<ip_connector>(bot_.get_service(), "127.0.0.1", service, true, false);

	// 1. Add all commands.
	for (const auto& f : daemon::transport_command::registry())
		bot_.transports().get_commands().push_back(f());

	// 2. Create controller and transport server.
	ctl_ = std::make_unique<ctl::controller>(std::move(connector));
	bot_.transports().add(std::make_unique<daemon::transport_server>(std::move(acceptor)));

	// 3. Wait for controller to connect.
	asio::deadline_timer timer(ctx_);

	timer.expires_from_now(posix_time::seconds(10));
	timer.async_wait([] (auto code) {
		if (code && code != asio::error::operation_aborted)
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

	bot_.servers().add(server_);
	bot_.plugins().add(plugin_);
	server_->disconnect();
	server_->clear();
	plugin_->clear();
}

} // !irccd::test
