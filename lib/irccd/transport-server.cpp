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

#include "sysconfig.h"

#if !defined(IRCCD_SYSTEM_WINDOWS)
#  include <cstdio>
#endif

#include <sstream>

#include "transport-server.hpp"

namespace irccd {

/*
 * TransportServerIp
 * ------------------------------------------------------------------
 */

TransportServerIp::TransportServerIp(int domain, const std::string &address, int port, bool ipv6only)
	: m_socket(domain, SOCK_STREAM, 0)
{
	m_socket.set(net::option::SockReuseAddress{true});

	/* Disable or enable IPv4 when using IPv6 */
	if (domain == AF_INET6)
		m_socket.set(net::option::Ipv6Only{ipv6only});

	m_socket.bind(net::address::Ip{address, port, static_cast<net::address::Ip::Type>(domain)});
	m_socket.listen();

	log::info() << "transport: listening on " << address << ", port " << port << std::endl;
}

net::Handle TransportServerIp::handle() noexcept
{
	return m_socket.handle();
}

std::shared_ptr<TransportClient> TransportServerIp::accept()
{
	return std::make_shared<TransportClientBase<net::address::Ip>>(m_socket.accept(nullptr));
}

/*
 * TransportServerUnix
 * ------------------------------------------------------------------
 */

#if !defined(IRCCD_SYSTEM_WINDOWS)

TransportServerUnix::TransportServerUnix(std::string path)
	: m_path(std::move(path))
{
	m_socket.bind(net::address::Local{m_path, true});
	m_socket.listen();

	log::info() << "transport: listening on " << m_path << std::endl;
}

TransportServerUnix::~TransportServerUnix()
{
	::remove(m_path.c_str());
}

net::Handle TransportServerUnix::handle() noexcept
{
	return m_socket.handle();
}

std::shared_ptr<TransportClient> TransportServerUnix::accept()
{
	return std::make_shared<TransportClientBase<net::address::Local>>(m_socket.accept(nullptr));
}

#endif

} // !irccd
