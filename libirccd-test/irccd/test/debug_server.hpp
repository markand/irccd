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

namespace irccd {

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
    void connect() noexcept override;

    /**
     * \copydoc server::connect
     */
    void disconnect() noexcept override;

    /**
     * \copydoc server::reconnect
     */
    void reconnect() noexcept override;

    /**
     * \copydoc server::invite
     */
    void invite(std::string target, std::string channel) override;

    /**
     * \copydoc server::join
     */
    void join(std::string channel, std::string password = "") override;

    /**
     * \copydoc server::kick
     */
    void kick(std::string target, std::string channel, std::string reason = "") override;

    /**
     * \copydoc server::me
     */
    void me(std::string target, std::string message) override;

    /**
     * \copydoc server::message
     */
    void message(std::string target, std::string message) override;

    /**
     * \copydoc server::mode
     */
    void mode(std::string channel,
              std::string mode,
              std::string limit = "",
              std::string user = "",
              std::string mask = "") override;

    /**
     * \copydoc server::names
     */
    void names(std::string channel) override;

    /**
     * \copydoc server::notice
     */
    void notice(std::string target, std::string message) override;

    /**
     * \copydoc server::part
     */
    void part(std::string channel, std::string reason = "") override;

    /**
     * \copydoc server::send
     */
    void send(std::string raw) override;

    /**
     * \copydoc server::topic
     */
    void topic(std::string channel, std::string topic) override;

    /**
     * \copydoc server::whois
     */
    void whois(std::string target) override;
};

} // !irccd

#endif // !IRCCD_TEST_DEBUG_SERVER_HPP
