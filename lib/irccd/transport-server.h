/*
 * transport-server.h -- I/O for irccd clients (acceptors)
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

#ifndef _IRCCD_TRANSPORT_SERVER_H_
#define _IRCCD_TRANSPORT_SERVER_H_

/**
 * @file transport-server.h
 * @brief Transports for irccd
 */

#include <memory>
#include <string>

#include <irccd-config.h>

#include "sockets.h"
#include "transport-client.h"

namespace irccd {

/**
 * @class TransportServer
 * @brief Bring networking between irccd and irccdctl
 *
 * This class contains a master sockets for listening to TCP connections, it is then processed by irccd.
 *
 * The transport class supports the following domains:
 *
 * | Domain                | Class                 |
 * |-----------------------|-----------------------|
 * | IPv4, IPv6            | TransportServerIp     |
 * | Unix (not on Windows) | TransportServerUnix   |
 *
 * Note: IPv4 and IPv6 can be combined, using TransportServer::IPv6 and its option.
 */
class TransportServer {
private:
	TransportServer(const TransportServer &) = delete;
	TransportServer(TransportServer &&) = delete;

	TransportServer &operator=(const TransportServer &) = delete;
	TransportServer &operator=(TransportServer &&) = delete;

public:
	/**
	 * Default constructor.
	 */
	TransportServer() = default;

	/**
	 * Destructor defaulted.
	 */
	virtual ~TransportServer() = default;

	/**
	 * Retrieve the underlying socket handle.
	 *
	 * @return the socket
	 */
	virtual net::Handle handle() noexcept = 0;

	/**
	 * Accept a new client depending on the domain.
	 *
	 * @return the new client
	 */
	virtual std::shared_ptr<TransportClient> accept() = 0;
};

/**
 * @class TransportServerIp
 * @brief Base class for both IPv4 and IPv6 servers.
 */
class TransportServerIp : public TransportServer {
protected:
	net::SocketTcp<net::address::Ip> m_socket;

public:
	/**
	 * Create a IP transport, use IPv6 or IPv4 address.
	 *
	 * @param domain AF_INET or AF_INET6
	 * @param address the address or "*" for any
	 * @param port the port number
	 * @param ipv6only set to true to disable IPv4
	 * @throw net::Error on failures
	 */
	TransportServerIp(int domain, const std::string &address, int port, bool ipv6only = true);

	/**
	 * @copydoc TransportServer::socket
	 */
	net::Handle handle() noexcept override;

	/**
	 * @copydoc TransportServer::accept
	 */
	std::shared_ptr<TransportClient> accept() override;
};

#if !defined(IRCCD_SYSTEM_WINDOWS)

/**
 * @class TransportServerUnix
 * @brief Implementation of transports for Unix sockets.
 */
class TransportServerUnix : public TransportServer {
private:
	net::SocketTcp<net::address::Local> m_socket;
	std::string m_path;

public:
	/**
	 * Create a Unix transport.
	 *
	 * @param path the path
	 */
	TransportServerUnix(std::string path);

	/**
	 * Destroy the transport and remove the file.
	 */
	~TransportServerUnix();

	/**
	 * @copydoc TransportServer::socket
	 */
	net::Handle handle() noexcept override;

	/**
	 * @copydoc TransportServer::accept
	 */
	std::shared_ptr<TransportClient> accept() override;
};

#endif // !_WIN32

} // !irccd

#endif // !_IRCCD_TRANSPORT_SERVER_H_