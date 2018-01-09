/*
 * connection.hpp -- abstract connection for irccdctl
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

#ifndef IRCCD_CTL_CONNECTION_HPP
#define IRCCD_CTL_CONNECTION_HPP

/**
 * \file connection.hpp
 * \brief Abstract connection for irccdctl.
 */

#include <boost/system/error_code.hpp>

#include <json.hpp>

namespace irccd {

namespace ctl {

/**
 * \brief Abstract connection for irccdctl.
 */
class connection {
public:
    /**
     * Connect handler.
     *
     * Call the handler when the underlying protocol connection is complete.
     */
    using connect_t = std::function<void (boost::system::error_code)>;

    /**
     * Tells if operations are in progress.
     */
    virtual bool is_active() const noexcept = 0;

    /**
     * Connect to the daemon.
     *
     * \param handler the non-null handler
     */
    virtual void connect(connect_t handler) = 0;

    /**
     * Receive from irccd.
     *
     * \param handler the completion handler
     */
    virtual void recv(network_recv_handler handler) = 0;

    /**
     * Send to irccd.
     *
     * \param json the json object
     * \param handler the completion handler
     */
    virtual void send(nlohmann::json json, network_send_handler handler) = 0;
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_CONNECTION_HPP
