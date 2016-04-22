/*
 * server-state.hpp -- server current state
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

#ifndef IRCCD_SERVER_STATE_HPP
#define IRCCD_SERVER_STATE_HPP

/**
 * \file server-state.hpp
 * \brief Server state.
 */

#include "elapsed-timer.hpp"
#include "sockets.hpp"
#include "sysconfig.hpp"

namespace irccd {

class Server;

/**
 * \class ServerState
 * \brief Server current state.
 */
class ServerState {
public:
	/**
	 * \enum Type
	 * \brief Server state
	 */
	enum Type {
		Undefined,	//!< Not defined yet
		Connecting,	//!< Connecting to the server
		Connected,	//!< Connected and running
		Disconnected,	//!< Disconnected and waiting before retrying
	};

private:
	Type m_type;

	/* For ServerState::Connecting */
	bool m_started{false};
	ElapsedTimer m_timer;

	/* Private helpers */
	bool connect(Server &server);

	/* Different preparation */
	void prepareConnected(Server &, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd);
	void prepareConnecting(Server &, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd);
	void prepareDisconnected(Server &, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd);

public:
	/**
	 * Create the server state.
	 *
	 * \pre type must be valid
	 * \param type the type
	 */
	ServerState(Type type);

	/**
	 * Prepare the state.
	 *
	 * \param server the server
	 * \param setinput the read set
	 * \param setoutput the write set
	 * \param maxfd the maximum fd
	 */
	void prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd);

	/**
	 * Get the state type.
	 *
	 * \return the type
	 */
	inline Type type() const noexcept
	{
		return m_type;
	}
};

} // !irccd

#endif // !IRCCD_SERVER_STATE_HPP
