/*
 * cli_test.cpp -- test fixture for irccdctl frontend
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

#include <irccd/string_util.hpp>
#include <irccd/socket_acceptor.hpp>

#include <irccd/daemon/service/transport_service.hpp>

#include "cli_test.hpp"

namespace proc = boost::process;

namespace irccd {

cli_test::cli_test()
{
    std::remove(CMAKE_BINARY_DIR "/tmp/irccd.sock");

    io::local_acceptor::endpoint endpoint(CMAKE_BINARY_DIR "/tmp/irccd.sock");
    io::local_acceptor::acceptor acceptor(service_, std::move(endpoint));

    irccd_.transports().add(std::make_unique<transport_server>(
        std::make_unique<io::local_acceptor>(std::move(acceptor))));
}

cli_test::~cli_test()
{
    service_.stop();
    thread_.join();
}

void cli_test::start()
{
    thread_ = std::thread([this] { service_.run(); });

    // Let irccd bind correctly.
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

cli_test::outputs cli_test::exec(const std::vector<std::string>& args)
{
    static const std::string irccdctl = IRCCDCTL_EXECUTABLE;
    static const std::string conf = CMAKE_BINARY_DIR "/tmp/irccdctl.conf";

    std::ostringstream oss;

    oss << irccdctl << " -c " << conf << " ";
    oss << string_util::join(args, " ");

    proc::ipstream out;
    proc::ipstream err;

    proc::system(
        oss.str(),
        proc::std_in.close(),
        proc::std_out > out,
        proc::std_err > err
    );

    outputs result;

    for (std::string line; out && std::getline(out, line); )
        result.first.push_back(line);
    for (std::string line; err && std::getline(err, line); )
        result.second.push_back(line);

    return result;
}

} // !irccd
