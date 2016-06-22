/*
 * service-server.hpp -- manage IRC servers
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_SERVICE_SERVER_HPP
#define IRCCD_SERVICE_SERVER_HPP

/**
 * \file service-server.hpp
 * \brief Manage IRC servers.
 */

#include <memory>
#include <set>
#include <string>

#include "server.hpp"
#include "service.hpp"

namespace irccd {

class Irccd;

/**
 * \brief Manage IRC servers.
 * \ingroup services
 */
class ServerService : public Service {
private:
    Irccd &m_irccd;
    std::vector<std::shared_ptr<Server>> m_servers;

    void handleChannelMode(const ChannelModeEvent &);
    void handleChannelNotice(const ChannelNoticeEvent &);
    void handleConnect(const ConnectEvent &);
    void handleInvite(const InviteEvent &);
    void handleJoin(const JoinEvent &);
    void handleKick(const KickEvent &);
    void handleMessage(const MessageEvent &);
    void handleMe(const MeEvent &);
    void handleMode(const ModeEvent &);
    void handleNames(const NamesEvent &);
    void handleNick(const NickEvent &);
    void handleNotice(const NoticeEvent &);
    void handlePart(const PartEvent &);
    void handleQuery(const QueryEvent &);
    void handleTopic(const TopicEvent &);
    void handleWhois(const WhoisEvent &);

public:
    /**
     * Create the server service.
     */
    IRCCD_EXPORT ServerService(Irccd &instance);

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max) override;

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out) override;

    /**
     * Get the list of servers
     *
     * \return the servers
     */
    inline const std::vector<std::shared_ptr<Server>> &servers() const noexcept
    {
        return m_servers;
    }

    /**
     * Check if a server exists.
     *
     * \param name the name
     * \return true if exists
     */
    IRCCD_EXPORT bool has(const std::string &name) const noexcept;

    /**
     * Add a new server to the application.
     *
     * \pre hasServer must return false
     * \param sv the server
     */
    IRCCD_EXPORT void add(std::shared_ptr<Server> sv);

    /**
     * Get a server or empty one if not found
     *
     * \param name the server name
     * \return the server or empty one if not found
     */
    IRCCD_EXPORT std::shared_ptr<Server> get(const std::string &name) const noexcept;

    /**
     * Find a server by name.
     *
     * \param name the server name
     * \return the server
     * \throw std::out_of_range if the server does not exist
     */
    IRCCD_EXPORT std::shared_ptr<Server> require(const std::string &name) const;

    /**
     * Remove a server from the irccd instance.
     *
     * The server if any, will be disconnected.
     *
     * \param name the server name
     */
    IRCCD_EXPORT void remove(const std::string &name);

    /**
     * Remove all servers.
     *
     * All servers will be disconnected.
     */
    IRCCD_EXPORT void clear() noexcept;
};

} // !irccd

#endif // !IRCCD_SERVICE_SERVER_HPP
