/*
 * command_fixture.hpp -- test fixture helper for transport commands
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

#ifndef IRCCD_TEST_COMMAND_FIXTURE_HPP
#define IRCCD_TEST_COMMAND_FIXTURE_HPP

/**
 * \file command_fixture.hpp
 * \brief Test fixture helper for transport commands.
 */

#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/rule_service.hpp>
#include <irccd/daemon/server_service.hpp>

#include <irccd/ctl/controller.hpp>

#include "irccd_fixture.hpp"
#include "mock_server.hpp"
#include "mock_plugin.hpp"

namespace irccd::test {

/**
 * \brief Test fixture helper for transport commands.
 *
 * This fixture automatically adds a mock_server and mock_plugin named "test"
 * and added to the respective services.
 */
class command_fixture : public irccd_fixture {
protected:
	/**
	 * \brief Result for request function.
	 */
	using result = std::pair<nlohmann::json, std::error_code>;

	/**
	 * Block for the next message.
	 *
	 * \param timer the timer to cancel on completion
	 * \return the next message
	 */
	auto recv(boost::asio::deadline_timer& timer) -> result;

	/**
	 * Block for the next command with a maximum timeout.
	 *
	 * \return the result
	 * \throw std::runtime_error after 30 seconds
	 */
	auto wait_command(const std::string& cmd) -> result;

	/**
	 * \brief Irccd controller
	 */
	std::unique_ptr<ctl::controller> ctl_;

	/**
	 * \brief Mock server object.
	 */
	std::shared_ptr<mock_server> server_;

	/**
	 * \brief Mock plugin object.
	 */
	std::shared_ptr<mock_plugin> plugin_;

	/**
	 * Constructor.
	 */
	command_fixture();

	/**
	 * Get result from irccd.
	 *
	 * \param json the request
	 * \return the result/error pair
	 */
	auto request(nlohmann::json json) -> result;
};

} // !irccd::test

#endif // !IRCCD_TEST_COMMAND_FIXTURE_HPP
