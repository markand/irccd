/*
 * controller.hpp -- main irccdctl interface
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

#ifndef IRCCD_CTL_CONTROLLER_HPP
#define IRCCD_CTL_CONTROLLER_HPP

/**
 * \file controller.hpp
 * \brief Main irccdctl interface.
 */

#include "sysconfig.hpp"

#include <cassert>

#include <irccd/connector.hpp>
#include <irccd/stream.hpp>

namespace irccd {

/**
 * \brief Namespace for irccdctl utilities.
 */
namespace ctl {

/**
 * \brief Main irccdctl interface.
 *
 * This class is an easy frontend to issue commands to irccd, it uses an
 * independant connection to perform the requests.
 *
 * This class is responsible of doing initial connection, performing checks and
 * optional authentication.
 *
 * It is implemented in mind that connection are asynchronous even though this
 * is not necessary.
 */
class controller {
public:
	/**
	 * Connection completion handler.
	 *
	 * This callback is called when connection has been completed or failed.
	 * In both case, the error code is set and the JSON object may contain
	 * the * irccd program information.
	 */
	using connect_handler = std::function<void (std::error_code, nlohmann::json)>;

private:
	std::unique_ptr<connector> connector_;
	std::shared_ptr<stream> stream_;
	std::string password_;

	void authenticate(connect_handler, nlohmann::json);
	void verify(connect_handler);

public:
	/**
	 * Construct the controller with its connection.
	 *
	 * \pre connector != nullptr
	 * \note no connect attempt is done
	 * \param connector the abstract connector
	 */
	controller(std::unique_ptr<connector> connector) noexcept;

	/**
	 * Get the optional password set.
	 *
	 * \return the password
	 */
	auto get_password() const noexcept -> const std::string&;

	/**
	 * Set an optional password.
	 *
	 * An empty password means no authentication (default).
	 *
	 * \param password the password
	 * \note this must be called before connect
	 */
	void set_password(std::string password) noexcept;

	/**
	 * Attempt to connect to the irccd daemon.
	 *
	 * \pre another connect operation must not be running
	 * \pre handler != nullptr
	 * \param handler the handler
	 */
	void connect(connect_handler handler);

	/**
	 * Request a message.
	 *
	 * \pre another recv operation must not be running
	 * \pre handler != nullptr
	 * \param handler the recv handler
	 */
	void recv(stream::recv_handler handler);

	/**
	 * Send a message.
	 *
	 * \pre another send operation must not be running
	 * \pre message.is_object()
	 * \pre handler != nullptr
	 * \param message the JSON message
	 * \param handler the optional completion handler
	 */
	void send(nlohmann::json message, stream::send_handler handler);
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_CONTROLLER_HPP
