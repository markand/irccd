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
 * TransportServerIpv6
 * ------------------------------------------------------------------
 */

TransportServerIpv6::TransportServerIpv6(const std::string &address, std::uint16_t port, bool ipv6only)
    : TransportServer(net::TcpSocket(AF_INET6, 0))
{
    m_socket.set(net::option::SockReuseAddress(true));

    // Disable or enable IPv4 when using IPv6.
    if (ipv6only)
        m_socket.set(net::option::Ipv6Only(true));

    if (address == "*")
        m_socket.bind(net::ipv6::any(port));
    else
        m_socket.bind(net::ipv6::pton(address, port));

    m_socket.listen();

    log::info() << "transport: listening on " << address << ", port " << port << std::endl;
}

/*
 * TransportServerIp
 * ------------------------------------------------------------------
 */

TransportServerIp::TransportServerIp(const std::string &address, std::uint16_t port)
    : TransportServer(net::TcpSocket(AF_INET, 0))
{
    m_socket.set(net::option::SockReuseAddress(true));

    if (address == "*")
        m_socket.bind(net::ipv4::any(port));
    else
        m_socket.bind(net::ipv4::pton(address, port));

    m_socket.listen();

    log::info() << "transport: listening on " << address << ", port " << port << std::endl;
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
