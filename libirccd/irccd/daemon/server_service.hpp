/*
 * server_service.hpp -- server service
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

#ifndef IRCCD_DAEMON_SERVER_SERVICE_HPP
#define IRCCD_DAEMON_SERVER_SERVICE_HPP

/**
 * \file server_service.hpp
 * \brief Server service.
 */

#include <memory>
#include <system_error>
#include <vector>

#include "server.hpp"

namespace irccd {

class config;
class irccd;

/**
 * \brief Manage IRC servers.
 * \ingroup services
 */
class server_service {
private:
	irccd& irccd_;
	std::vector<std::shared_ptr<server>> servers_;

	void handle_error(const std::shared_ptr<server>&, const std::error_code&);
	void handle_wait(const std::shared_ptr<server>&, const std::error_code&);
	void handle_recv(const std::shared_ptr<server>&, const std::error_code&, const event&);
	void handle_connect(const std::shared_ptr<server>&, const std::error_code&);

	void wait(const std::shared_ptr<server>&);
	void recv(const std::shared_ptr<server>&);
	void connect(const std::shared_ptr<server>&);

public:
	/**
	 * Create the server service.
	 */
	server_service(irccd& instance);

	/**
	 * Get the list of servers
	 *
	 * \return the servers
	 */
	auto all() const noexcept -> const std::vector<std::shared_ptr<server>>&;

	/**
	 * Check if a server exists.
	 *
	 * \param name the name
	 * \return true if exists
	 */
	auto has(const std::string& name) const noexcept -> bool;

	/**
	 * Add a new server to the application.
	 *
	 * \pre hasServer must return false
	 * \param sv the server
	 */
	void add(std::shared_ptr<server> sv);

	/**
	 * Get a server or empty one if not found
	 *
	 * \param name the server name
	 * \return the server or empty one if not found
	 */
	auto get(std::string_view name) const noexcept -> std::shared_ptr<server>;

	/**
	 * Find a server from a JSON object.
	 *
	 * \param name the server name
	 * \return the server
	 * \throw server_error on errors
	 */
	auto require(std::string_view name) const -> std::shared_ptr<server>;

	/**
	 * Force disconnection, this also call plugin::handle_disconnect handler.
	 *
	 * \param id the server id
	 * \throw server_error on errors
	 */
	void disconnect(std::string_view id);

	/**
	 * Force reconnection, this also call plugin::handle_disconnect handler.
	 *
	 * \param id the server id
	 * \return the server
	 * \throw server_error on errors
	 */
	void reconnect(std::string_view id);

	/**
	 * Force reconnection of all servers.
	 */
	void reconnect();

	/**
	 * Remove a server from the irccd instance.
	 *
	 * The server if any, will be disconnected.
	 *
	 * \param name the server name
	 */
	void remove(std::string_view name);

	/**
	 * Remove all servers.
	 *
	 * All servers will be disconnected.
	 */
	void clear() noexcept;

	/**
	 * Load servers from the configuration.
	 *
	 * \param cfg the config
	 */
	void load(const config& cfg) noexcept;
};

namespace logger {

template <typename T>
struct loggable_traits;

template <>
struct loggable_traits<server> {
	static auto get_category(const server& server) -> std::string_view;

	static auto get_component(const server& server) -> std::string_view;
};

} // !logger

} // !irccd

#endif // !IRCCD_DAEMON_SERVER_SERVICE_HPP
