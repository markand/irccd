/*
 * service-transport.hpp -- manage transport servers and clients
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

#ifndef IRCCD_SERVICE_TRANSPORT_HPP
#define IRCCD_SERVICE_TRANSPORT_HPP

/**
 * \file service-transport.hpp
 * \brief manage transport servers and clients.
 */

#include <json.hpp>

#include "service.hpp"

namespace irccd {

class TransportServer;
class TransportClient;

/**
 * \brief manage transport servers and clients.
 * \ingroup services
 */
class TransportService : public Service {
private:
    Irccd &m_irccd;

    std::vector<std::shared_ptr<TransportServer>> m_servers;
    std::vector<std::shared_ptr<TransportClient>> m_clients;

    void handleCommand(std::weak_ptr<TransportClient>, const nlohmann::json &);
    void handleDie(std::weak_ptr<TransportClient>);

public:
    /**
     * Create the transport service.
     *
     * \param irccd the irccd instance
     */
    IRCCD_EXPORT TransportService(Irccd &irccd) noexcept;

    /**
     * \copydoc Service::prepare
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max) override;

    /**
     * \copydoc Service::sync
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out) override;

    /**
     * Add a transport server.
     *
     * \param ts the transport server
     */
    IRCCD_EXPORT void add(std::shared_ptr<TransportServer> ts);

    /**
     * Send data to all clients.
     *
     * \param data the data
     */
    IRCCD_EXPORT void broadcast(std::string data);
};

} // !irccd

#endif // !IRCCD_SERVICE_TRANSPORT_HPP
