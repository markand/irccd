/*
 * cli_fixture.hpp -- test fixture for irccdctl frontend
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

#ifndef IRCCD_TEST_CLI_FIXTURE_HPP
#define IRCCD_TEST_CLI_FIXTURE_HPP

/**
 * \file cli_fixture.hpp
 * \brief Test fixture for irccdctl frontend.
 */

#include <thread>
#include <tuple>
#include <vector>

#include <boost/asio.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/hook_service.hpp>
#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/rule_service.hpp>
#include <irccd/daemon/server_service.hpp>

#include <irccd/test/mock_server.hpp>

namespace irccd::test {

/**
 * \brief Test fixture for irccdctl frontend.
 *
 * This class will run irccd daemon in a thread when member function `start` is
 * called.
 *
 * Before starting the daemon, the test can manually modify irccd instance
 * through `bot_` member variable. Once started, call `exec` with arguments
 * you want to pass through irccdctl utility.
 */
class cli_fixture {
private:
	std::string irccdctl_;
	std::thread thread_;
	std::uint16_t port_{0U};

	boost::asio::io_context service_;

protected:
	/**
	 * Irccd instance.
	 *
	 * \warning Do not modify once `start()` has been called.
	 */
	daemon::bot bot_{service_};

	/**
	 * Server automatically added as "test".
	 */
	std::shared_ptr<mock_server> server_;

public:
	/**
	 * Type for all lines printed.
	 */
	using outputs = std::vector<std::string>;

	/**
	 * Collection of output from stdout/stderr respectively.
	 */
	using result = std::tuple<int, outputs, outputs>;

	/**
	 * Construct and initialize and irccd daemon running in a thread.
	 *
	 * \param irccdctl path to irccdctl
	 */
	cli_fixture(std::string irccdctl);

	/**
	 * Stop irccd and close everything.
	 */
	~cli_fixture();

	/**
	 * Start irccd daemon.
	 *
	 * A thread will be running and closed when the destructor is called, you
	 * MUST not modify irccd while running.
	 */
	void start();

	/**
	 * Execute irccdctl.
	 *
	 * \param args the arguments to irccdctl
	 * \return the stdout/stderr and exit code
	 */
	auto exec(const std::vector<std::string>& args) -> result;
};

} // !irccd::test

#endif // !IRCCD_TEST_CLI_FIXTURE_HPP
