/*
 * transport-server.cpp -- I/O for irccd clients (acceptors)
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

#include "sysconfig.hpp"

#if !defined(IRCCD_SYSTEM_WINDOWS)
#  include <cstdio>
#endif

#include <sstream>

#include "logger.hpp"
#include "transport-server.hpp"

namespace irccd {

/*
 * TransportServerIp
 * ------------------------------------------------------------------
 */

TransportServerIp::TransportServerIp(const std::string &address,
                                     std::uint16_t port,
                                     std::uint8_t mode)
    : TransportServer(net::TcpSocket((mode & v6) ? AF_INET6 : AF_INET, 0))
{
    assert((mode & v6) || (mode & v4));

    m_socket.set(net::option::SockReuseAddress(true));

    if (mode & v6) {
        if (address == "*")
            m_socket.bind(net::ipv6::any(port));
        else
            m_socket.bind(net::ipv6::pton(address, port));

        // Disable or enable IPv4 when using IPv6.
        if (!(mode & v4))
            m_socket.set(net::option::Ipv6Only(true));
    } else {
        if (address == "*")
            m_socket.bind(net::ipv4::any(port));
        else
            m_socket.bind(net::ipv4::pton(address, port));
    }

    m_socket.listen();

    log::info() << "transport: listening on " << address << ", port " << port << std::endl;
}

/*
 * TransportServerTls
 * ------------------------------------------------------------------
 */

TransportServerTls::TransportServerTls(const std::string &pkey,
                                       const std::string &cert,
                                       const std::string &address,
                                       std::uint16_t port,
                                       std::uint8_t mode)
    : TransportServerIp(address, port, mode)
    , m_privatekey(pkey)
    , m_cert(cert)
{
    log::info() << "transport: listening on " << address << ", port " << port
                << " (using SSL)" << std::endl;
}

std::unique_ptr<TransportClient> TransportServerTls::accept()
{
    return std::make_unique<TransportClientTls>(m_privatekey, m_cert, m_socket.accept());
}

/*
 * TransportServerUnix
 * ------------------------------------------------------------------
 */

#if !defined(IRCCD_SYSTEM_WINDOWS)

TransportServerLocal::TransportServerLocal(std::string path)
    : TransportServer(net::TcpSocket(AF_LOCAL, 0))
    , m_path(std::move(path))
{
    m_socket.bind(net::local::create(m_path, true));
    m_socket.listen();

    log::info() << "transport: listening on " << m_path << std::endl;
}

TransportServerLocal::~TransportServerLocal()
{
    ::remove(m_path.c_str());
}

#endif

} // !irccd
