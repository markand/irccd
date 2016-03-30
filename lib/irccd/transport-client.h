/*
 * transport-client.h -- client connected to irccd
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

#ifndef _IRCCD_TRANSPORT_CLIENT_H_
#define _IRCCD_TRANSPORT_CLIENT_H_

/**
 * @file TransportClient.h
 * @brief Client connected to irccd
 */

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include "server.h"
#include "signals.h"
#include "sockets.h"

namespace irccd {

namespace json {

class Value;

} // !json

/**
 * @class TransportClient
 * @brief Client connected to irccd.
 *
 * This class emits a warning upon clients request through onCommand signal.
 */
class TransportClient {
public:
	/**
	 * Signal: onCommand
	 * ----------------------------------------------------------
	 *
	 * Arguments:
	 *   - the command
	 */
	Signal<const json::Value &> onCommand;

	/**
	 * Signal: onDie
	 * ----------------------------------------------------------
	 *
	 * The client has disconnected.
	 */
	Signal<> onDie;

protected:
	std::string m_input;
	std::string m_output;

	/* Parse input buffer */
	void parse(const std::string &);

	/* Do I/O */
	virtual void receive() = 0;
	virtual void send() = 0;

public:
	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~TransportClient() = default;

	/**
	 * Send or receive data, called after a select.
	 *
	 * @param setinput the input fd_set
	 * @param setoutput the output fd_set
	 */
	void sync(fd_set &setinput, fd_set &setoutput);

#if 0
	/**
	 * Notify the client that the command succeeded.
	 *
	 * @param command the command name
	 */
	void ok(const std::string &command);

	/**
	 * Send an error message to the client.
	 *
	 * @param command the command name
	 * @param message the error message
	 */
	void error(const std::string &command, std::string message);
#endif

	/**
	 * Send some data, it will be pushed to the outgoing buffer.
	 *
	 * This function appends "\r\n\r\n" after the message so you don't have
	 * to do it manually.
	 *
	 * @param message the message
	 */
	void send(std::string message);

	/**
	 * Tell if the client has data pending for output.
	 *
	 * @return true if has pending data to write
	 */
	inline bool hasOutput() const noexcept
	{
		return !m_output.empty();
	}

	/**
	 * Get the underlying socket handle.
	 *
	 * @return the socket
	 */
	virtual net::Handle handle() noexcept = 0;
};

/**
 * @class TransportClient
 * @brief Template class for Tcp and Ssl sockets
 */
template <typename Address>
class TransportClientBase : public TransportClient {
private:
	net::SocketTcp<Address> m_socket;

protected:
	void send() override;
	void receive() override;

public:
	/**
	 * Create a client.
	 *
	 * @param sock the socket
	 */
	inline TransportClientBase(net::SocketTcp<Address> socket)
		: m_socket(std::move(socket))
	{
	}

	/**
	 * @copydoc TransportClient::handle
	 */
	net::Handle handle() noexcept override
	{
		return m_socket.handle();
	}
};

template <typename Address>
void TransportClientBase<Address>::receive()
{
	try {
		auto message = m_socket.recv(512);

		if (message.empty())
			onDie();

		m_input += message;
	} catch (const std::exception &) {
		onDie();
	}

	std::string::size_type pos;
	while ((pos = m_input.find("\r\n\r\n")) != std::string::npos) {
		/*
		 * Make a copy and erase it in case that onComplete function
		 * throws.
		 */
		auto message = m_input.substr(0, pos);

		m_input.erase(m_input.begin(), m_input.begin() + pos + 4);

		parse(message);
	}
}

template <typename Address>
void TransportClientBase<Address>::send()
{
	m_output.erase(0, m_socket.send(m_output));
}

} // !irccd

#endif // !_IRCCD_TRANSPORT_CLIENT_H_
