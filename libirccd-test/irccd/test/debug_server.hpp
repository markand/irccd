/*
 * debug_server.hpp -- server which prints everything in the console
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

#ifndef IRCCD_TEST_DEBUG_SERVER_HPP
#define IRCCD_TEST_DEBUG_SERVER_HPP

/**
 * \file debug_server.hpp
 * \brief Server which prints everything in the console.
 */

#include <irccd/daemon/server.hpp>

namespace irccd::test {

/**
 * \brief Server which prints everything in the console.
 */
class debug_server : public server {
public:
	/**
	 * Inherited constructors.
	 */
	using server::server;

	/**
	 * \copydoc server::connect
	 */
	void connect(connect_handler handler) noexcept override;

	/**
	 * \copydoc server::disconnect
	 */
	void disconnect() noexcept override;

	/**
	 * \copydoc server::invite
	 */
	void invite(std::string_view target, std::string_view channel) override;

	/**
	 * \copydoc server::join
	 */
	void join(std::string_view channel, std::string_view password = "") override;

	/**
	 * \copydoc server::kick
	 */
	void kick(std::string_view target, std::string_view channel, std::string_view reason = "") override;

	/**
	 * \copydoc server::me
	 */
	void me(std::string_view target, std::string_view message) override;

	/**
	 * \copydoc server::message
	 */
	void message(std::string_view target, std::string_view message) override;

	/**
	 * \copydoc server::mode
	 */
	void mode(std::string_view channel,
	          std::string_view mode,
	          std::string_view limit = "",
	          std::string_view user = "",
	          std::string_view mask = "") override;

	/**
	 * \copydoc server::names
	 */
	void names(std::string_view channel) override;

	/**
	 * \copydoc server::notice
	 */
	void notice(std::string_view target, std::string_view message) override;

	/**
	 * \copydoc server::part
	 */
	void part(std::string_view channel, std::string_view reason = "") override;

	/**
	 * \copydoc server::send
	 */
	void send(std::string_view raw) override;

	/**
	 * \copydoc server::topic
	 */
	void topic(std::string_view channel, std::string_view topic) override;

	/**
	 * \copydoc server::whois
	 */
	void whois(std::string_view target) override;
};

} // !irccd::test

#endif // !IRCCD_TEST_DEBUG_SERVER_HPP
