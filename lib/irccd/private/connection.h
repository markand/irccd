/*
 * connection.h -- value wrapper for connecting to irccd
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

#ifndef _IRCCD_CONNECTION_H_
#define _IRCCD_CONNECTION_H_

#include <cassert>
#include <stdexcept>

#include <irccd/json.h>
#include <irccd/system.h>
#include <irccd/util.h>

#include "elapsed-timer.h"
#include "sockets.h"

namespace irccd {

/**
 * @class Connection
 * @brief Abstract class for connecting to irccd from Ip or Local addresses.
 */
class Connection {
protected:
	ElapsedTimer m_timer;

	/**
	 * Clamp the time to wait to be sure that it will be never less than 0.
	 */
	inline int clamp(int timeout) noexcept
	{
		return timeout < 0 ? -1 : (timeout - (int)m_timer.elapsed() < 0) ? 0 : (timeout - m_timer.elapsed());
	}

public:
	/**
	 * Default constructor.
	 */
	Connection() = default;

	/**
	 * Default destructor.
	 */
	virtual ~Connection() = default;

	/**
	 * Wait for the next requested response.
	 *
	 * @param name the response name
	 * @param timeout the optional timeout
	 * @return the object
	 * @throw net::Error on errors or on timeout
	 */
	json::Value next(const std::string &name, int timeout = 30000);

	/**
	 * Just wait if the operation succeeded.
	 *
	 * @param name the response name
	 * @param timeout the timeout
	 */
	void verify(const std::string &name, int timeout = 30000);

	/**
	 * Check if the socket is still connected.
	 *
	 * @return true if connected
	 */
	virtual bool isConnected() const noexcept = 0;

	/**
	 * Try to connect to the host.
	 *
	 * @param timeout the maximum time in milliseconds
	 * @throw net::Error on errors or timeout
	 */
	virtual void connect(int timeout = 30000) = 0;

	/**
	 * Try to send the message in 30 seconds. The message must not end with \r\n\r\n, it is added automatically.
	 *
	 * @pre msg must not be empty
	 * @param msg the message to send
	 * @param timeout the maximum time in milliseconds
	 * @throw net::Error on errors
	 */
	virtual void send(std::string msg, int timeout = 30000) = 0;

	/**
	 * Get the next event from irccd.
	 *
	 * This functions throws if the connection is lost.
	 *
	 * @param timeout the maximum time in milliseconds
	 * @return the next event
	 * @throw net::Error on errors or disconnection
	 */
	virtual json::Value next(int timeout = 30000) = 0;
};

/**
 * @class ConnectionBase
 * @brief Implementation for Ip or Local.
 */
template <typename Address>
class ConnectionBase : public Connection {
private:
	net::SocketTcp<Address> m_socket;
	net::Listener<> m_listener;
	Address m_address;

	/* Input buffer */
	std::string m_input;

public:
	/**
	 * Construct the socket but do not connect immediately.
	 *
	 * @param address the address
	 */
	ConnectionBase(Address address)
		: m_address(std::move(address))
	{
		m_socket.set(net::option::SockBlockMode{false});
		m_listener.set(m_socket.handle(), net::Condition::Readable);
	}

	/**
	 * @copydoc Connection::isConnected
	 */
	bool isConnected() const noexcept override
	{
		return m_socket.state() == net::State::Connected;
	}

	/**
	 * @copydoc Connection::connect
	 */
	void connect(int timeout) override;

	/**
	 * @copydoc Connection::send
	 */
	void send(std::string msg, int timeout) override;

	/**
	 * @copydoc Connection::next
	 */
	json::Value next(int timeout) override;
};

template <typename Address>
void ConnectionBase<Address>::connect(int timeout)
{
	m_socket.connect(m_address);

	if (m_socket.state() == net::State::Connecting) {
		m_listener.set(m_socket.handle(), net::Condition::Writable);
		m_listener.wait(timeout);
		m_socket.connect();
		m_listener.unset(m_socket.handle(), net::Condition::Writable);
	}
}

template <typename Address>
void ConnectionBase<Address>::send(std::string msg, int timeout)
{
	assert(!msg.empty());

	/* Add termination */
	msg += "\r\n\r\n";

	m_listener.remove(m_socket.handle());
	m_listener.set(m_socket.handle(), net::Condition::Writable);
	m_timer.reset();

	while (!msg.empty()) {
		/* Do not wait the time that is already passed */
		m_listener.wait(clamp(timeout));

		/* Try to send at most as possible */
		msg.erase(0, m_socket.send(msg));
	}

	/* Timeout? */
	if (!msg.empty())
		throw std::runtime_error("operation timed out while sending to irccd");
}

template <typename Address>
json::Value ConnectionBase<Address>::next(int timeout)
{
	/* Maybe there is already something */
	std::string buffer = util::nextNetwork(m_input);

	m_listener.remove(m_socket.handle());
	m_listener.set(m_socket.handle(), net::Condition::Readable);
	m_timer.reset();

	/* Read if there is nothing */
	while (buffer.empty() && isConnected()) {
		/* Wait and read */
		m_listener.wait(clamp(timeout));
		m_input += m_socket.recv(512);

		/* Finally try */
		buffer = util::nextNetwork(m_input);
	}

	if (!isConnected())
		throw std::runtime_error("connection lost");

	json::Value value(json::Buffer{buffer});

	if (!value.isObject())
		throw std::invalid_argument("invalid message received");

	return value;
}

} // !irccd

#endif // !_IRCCD_CONNECTION_H_
