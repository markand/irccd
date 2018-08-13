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
#include <irccd/socket_acceptor.hpp>

#include <irccd/daemon/command.hpp>
#include <irccd/daemon/transport_service.hpp>
#include <irccd/daemon/transport_server.hpp>

#include "cli_fixture.hpp"

namespace proc = boost::process;

namespace irccd::test {

cli_fixture::cli_fixture()
    : server_(new mock_server(irccd_.get_service(), "test", "localhost"))
{
    std::remove(CMAKE_BINARY_DIR "/tmp/irccd.sock");

    io::local_acceptor::endpoint endpoint(CMAKE_BINARY_DIR "/tmp/irccd.sock");
    io::local_acceptor::acceptor acceptor(service_, std::move(endpoint));

    for (const auto& f : command::registry)
        irccd_.transports().get_commands().push_back(f());

    irccd_.servers().add(server_);
    irccd_.transports().add(std::make_unique<transport_server>(
        std::make_unique<io::local_acceptor>(std::move(acceptor))));
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
    static const std::string irccdctl = IRCCDCTL_EXECUTABLE;
    static const std::string conf = CMAKE_BINARY_DIR "/tmp/irccdctl.conf";

    std::ostringstream oss;

    oss << irccdctl << " -c " << conf << " ";
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
        out.push_back(line);
    for (std::string line; stream_err && std::getline(stream_err, line); )
        err.push_back(line);

    return { ret, out, err };
}

} // !irccd::test
