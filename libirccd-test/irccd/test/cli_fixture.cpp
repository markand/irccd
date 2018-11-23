/*
 * cli_fixture.cpp -- test fixture for irccdctl frontend
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

#include <chrono>
#include <sstream>

#include <boost/process.hpp>

#include <irccd/string_util.hpp>
#include <irccd/acceptor.hpp>

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/transport_service.hpp>
#include <irccd/daemon/transport_server.hpp>

#include "cli_fixture.hpp"
#include "test_plugin_loader.hpp"

namespace proc = boost::process;

using irccd::daemon::bot;
using irccd::daemon::command;
using irccd::daemon::transport_server;

namespace irccd::test {

namespace {

auto clear(std::string input) -> std::string
{
	while (input.size() > 0U && (input.back() == '\r' || input.back() == '\n'))
		input.pop_back();

	return input;
}

} // !namespace

cli_fixture::cli_fixture(std::string irccdctl)
	: irccdctl_(std::move(irccdctl))
	, server_(new mock_server(bot_.get_service(), "test", "localhost"))
{
	using boost::asio::ip::tcp;

	tcp::endpoint ep(tcp::v4(), 0U);
	tcp::acceptor raw_acceptor(bot_.get_service(), std::move(ep));

	port_ = raw_acceptor.local_endpoint().port();

	auto acceptor = std::make_unique<ip_acceptor>(bot_.get_service(), std::move(raw_acceptor));

	for (const auto& f : command::registry())
		bot_.transports().get_commands().push_back(f());

	bot_.servers().add(server_);
	bot_.transports().add(std::make_unique<transport_server>(std::move(acceptor)));
	bot_.plugins().add_loader(std::make_unique<test_plugin_loader>());
	server_->disconnect();
	server_->clear();
}

cli_fixture::~cli_fixture()
{
	service_.stop();
	thread_.join();
}

void cli_fixture::start()
{
	thread_ = std::thread([this] { service_.run(); });

	// Let irccd bind correctly.
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

auto cli_fixture::exec(const std::vector<std::string>& args) -> result
{
	std::ostringstream oss;

	oss << irccdctl_ << " -t ip --hostname 127.0.0.1 -p " << port_ << " ";
	oss << string_util::join(args, " ");

	proc::ipstream stream_out, stream_err;

	const auto ret = proc::system(
		oss.str(),
		proc::std_in.close(),
		proc::std_out > stream_out,
		proc::std_err > stream_err
	);

	outputs out, err;

	for (std::string line; stream_out && std::getline(stream_out, line); )
		out.push_back(clear(line));
	for (std::string line; stream_err && std::getline(stream_err, line); )
		err.push_back(clear(line));

	return { ret, out, err };
}

} // !irccd::test
