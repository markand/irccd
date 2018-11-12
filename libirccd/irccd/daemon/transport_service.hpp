/*
 * transport_service.hpp -- transport service
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

#ifndef IRCCD_DAEMON_TRANSPORT_SERVICE_HPP
#define IRCCD_DAEMON_TRANSPORT_SERVICE_HPP

/**
 * \file transport_service.hpp
 * \brief Transport service.
 */

/**
 * \defgroup transports Transports
 * \brief Transport client and servers
 */

#include <memory>
#include <vector>

#include <json.hpp>

namespace irccd {

class command;
class config;
class irccd;
class transport_client;
class transport_server;

/**
 * \brief Manage transport servers and clients.
 * \ingroup services
 * \ingroup transports
 */
class transport_service {
public:
	/**
	 * \brief the list of transport commands.
	 */
	using commands = std::vector<std::unique_ptr<command>>;

	/**
	 * \brief The list of transport acceptors.
	 */
	using servers = std::vector<std::shared_ptr<transport_server>>;

private:
	irccd& irccd_;
	commands commands_;
	servers servers_;

	void handle_command(std::shared_ptr<transport_client>, const nlohmann::json&);
	void do_recv(std::shared_ptr<transport_client>);
	void do_accept(transport_server&);

public:
	/**
	 * Create the transport service.
	 *
	 * \param irccd the irccd instance
	 */
	transport_service(irccd& irccd) noexcept;

	/**
	 * Default destructor.
	 */
	~transport_service() noexcept;

	/**
	 * Get underlying commands.
	 *
	 * \return the commands
	 */
	auto get_commands() const noexcept -> const commands&;

	/**
	 * Get underlying commands.
	 *
	 * \return the commands
	 */
	auto get_commands() noexcept -> commands&;

	/**
	 * Add a transport server.
	 *
	 * \param ts the transport server
	 */
	void add(std::shared_ptr<transport_server> ts);

	/**
	 * Send data to all clients.
	 *
	 * \pre object.is_object()
	 * \param object the json object
	 */
	void broadcast(const nlohmann::json& object);

	/**
	 * Load transports from the configuration.
	 *
	 * \param cfg the config
	 */
	void load(const config& cfg) noexcept;
};

} // !irccd

#endif // !IRCCD_DAEMON_TRANSPORT_SERVICE_HPP
