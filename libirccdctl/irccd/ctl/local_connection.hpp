/*
 * local_connection.hpp -- unix domain connection for irccdctl
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_CTL_LOCAL_CONNECTION_HPP
#define IRCCD_CTL_LOCAL_CONNECTION_HPP

/**
 * \file local_connection.hpp
 * \brief Unix domain connection for irccdctl.
 */

#include <irccd/sysconfig.hpp>

#include "network_connection.hpp"

#if !defined(IRCCD_SYSTEM_WINDOWS)

namespace irccd {

namespace ctl {

/**
 * \brief Unix domain connection for irccdctl.
 */
class local_connection : public network_connection<boost::asio::local::stream_protocol::socket> {
private:
    std::string path_;

public:
    /**
     * Construct the local connection.
     *
     * \param service the io_service
     * \param path the path to the socket file
     */
    inline local_connection(boost::asio::io_service& service, std::string path) noexcept
        : network_connection(service)
        , path_(std::move(path))
    {
    }

    /**
     * Connect to the socket file.
     *
     * \param handler the handler
     */
    void connect(connect_t handler) override;
};

} // !ctl

} // !irccd

#endif // !IRCCD_SYSTEM_WINDOWS

#endif // !IRCCD_CTL_LOCAL_CONNECTION_HPP
