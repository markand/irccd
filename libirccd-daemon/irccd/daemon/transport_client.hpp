/*
 * transport_client.hpp -- server side transport clients
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

#ifndef IRCCD_DAEMON_TRANSPORT_CLIENT_HPP
#define IRCCD_DAEMON_TRANSPORT_CLIENT_HPP

/**
 * \file transport_client.hpp
 * \brief Server side transport clients.
 */

#include <irccd/sysconfig.hpp>

#include <deque>
#include <memory>
#include <string_view>

#include <irccd/stream.hpp>

namespace irccd::daemon {

class transport_server;

/**
 * \brief Abstract transport client class.
 * \ingroup transports
 *
 * This class is responsible of receiving/sending data.
 */
class transport_client : public std::enable_shared_from_this<transport_client> {
public:
	/**
	 * Client state.
	 */
	enum class state {
		authenticating,         //!< client is authenticating
		ready,                  //!< client is ready
		closing                 //!< client is closing
	};

private:
	state state_{state::authenticating};
	std::weak_ptr<transport_server> parent_;
	std::shared_ptr<stream> stream_;
	std::deque<std::pair<nlohmann::json, stream::send_handler>> queue_;

	void flush();
	void erase();

public:
	/**
	 * Constructor.
	 *
	 * \pre stream != nullptr
	 * \param server the parent
	 * \param stream the I/O stream
	 */
	transport_client(std::weak_ptr<transport_server> server,
	                 std::shared_ptr<stream> stream) noexcept;

	/**
	 * Get the current client state.
	 *
	 * \return the state
	 */
	auto get_state() const noexcept -> state;

	/**
	 * Set the client state.
	 *
	 * \param state the new state
	 */
	void set_state(state state) noexcept;

	/**
	 * Start receiving if not closed.
	 *
	 * Possible error codes:
	 *
	 *   - std::errc::network_down in case of errors,
	 *   - std::errc::invalid_argument if the JSON message is invalid,
	 *   - std::errc::not_enough_memory in case of memory failure.
	 *
	 * \pre handler != nullptr
	 * \param handler the handler
	 * \warning Another read operation MUST NOT be running.
	 */
	void read(stream::recv_handler handler);

	/**
	 * Start sending if not closed.
	 *
	 * Possible error codes:
	 *
	 *   - boost::system::errc::network_down in case of errors,
	 *
	 * \pre json.is_object()
	 * \param json the json message
	 * \param handler the optional handler
	 * \note If a write operation is running, it is postponed once ready.
	 */
	void write(nlohmann::json json, stream::send_handler handler = nullptr);

	/**
	 * Convenient success message.
	 *
	 * \param command the command name
	 * \param handler the optional handler
	 * \note If a write operation is running, it is postponed once ready.
	 */
	void success(const std::string& command, stream::send_handler handler = nullptr);

	/**
	 * Send an error code to the client.
	 *
	 * \pre code is not 0
	 * \param code the error code
	 * \param handler the optional handler
	 * \note If a write operation is running, it is postponed once ready.
	 */
	void error(std::error_code code, stream::send_handler handler = nullptr);

	/**
	 * Send an error code to the client.
	 *
	 * \pre code is not 0
	 * \param code the error code
	 * \param command the command name
	 * \param handler the optional handler
	 * \note If a write operation is running, it is postponed once ready.
	 */
	void error(std::error_code code, std::string_view command, stream::send_handler handler = nullptr);
};

} // !irccd::daemon

#endif // !IRCCD_DAEMON_TRANSPORT_CLIENT_HPP
