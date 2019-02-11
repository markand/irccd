/*
 * command_fixture.hpp -- test fixture helper for transport commands
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

#ifndef IRCCD_TEST_COMMAND_FIXTURE_HPP
#define IRCCD_TEST_COMMAND_FIXTURE_HPP

/**
 * \file command_fixture.hpp
 * \brief Test fixture helper for transport commands.
 */

#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/rule_service.hpp>
#include <irccd/daemon/server_service.hpp>
#include <irccd/daemon/transport_client.hpp>

#include "irccd_fixture.hpp"
#include "mock_server.hpp"
#include "mock_plugin.hpp"
#include "mock_stream.hpp"

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
	 * \brief Mock server object.
	 */
	std::shared_ptr<mock_server> server_;

	/**
	 * \brief Mock plugin object.
	 */
	std::shared_ptr<mock_plugin> plugin_;

	/**
	 * \brief The fake transport_client stream.
	 */
	std::shared_ptr<mock_stream> stream_;
	/**
	 * \brief Client sending request.
	 */
	std::shared_ptr<daemon::transport_client> client_;

	/**
	 * Constructor.
	 */
	command_fixture();

	/**
	 * Get result from irccd.
	 *
	 * \param json the request
	 * \return the json message sent (if any)
	 */
	auto request(nlohmann::json json) -> nlohmann::json;
};

} // !irccd::test

#endif // !IRCCD_TEST_COMMAND_FIXTURE_HPP
