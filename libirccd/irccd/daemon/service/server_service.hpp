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
#include <vector>

#include <irccd/daemon/server.hpp>

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

    void handle_connect(const connect_event&);
    void handle_disconnect(const disconnect_event&);
    void handle_die(const disconnect_event&);
    void handle_invite(const invite_event&);
    void handle_join(const join_event&);
    void handle_kick(const kick_event&);
    void handle_message(const message_event&);
    void handle_me(const me_event&);
    void handle_mode(const mode_event&);
    void handle_names(const names_event&);
    void handle_nick(const nick_event&);
    void handle_notice(const notice_event&);
    void handle_part(const part_event&);
    void handle_query(const query_event&);
    void handle_topic(const topic_event&);
    void handle_whois(const whois_event&);

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
    inline const std::vector<std::shared_ptr<server>>& servers() const noexcept
    {
        return servers_;
    }

    /**
     * Check if a server exists.
     *
     * \param name the name
     * \return true if exists
     */
    bool has(const std::string& name) const noexcept;

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
    std::shared_ptr<server> get(const std::string& name) const noexcept;

    /**
     * Find a server from a JSON object.
     *
     * \param name the server name
     * \return the server
     * \throw server_error on errors
     */
    std::shared_ptr<server> require(const std::string& name) const;

    /**
     * Remove a server from the irccd instance.
     *
     * The server if any, will be disconnected.
     *
     * \param name the server name
     */
    void remove(const std::string& name);

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

} // !irccd

#endif // !IRCCD_DAEMON_SERVER_SERVICE_HPP
