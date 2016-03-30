/*
 * server-state.h -- server current state
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

#ifndef _IRCCD_SERVER_STATE_H_
#define _IRCCD_SERVER_STATE_H_

#include <irccd-config.h>

#include "elapsed-timer.h"
#include "sockets.h"

namespace irccd {

class Server;

/**
 * @class ServerState
 * @brief Server current state.
 */
class ServerState {
public:
	/**
	 * @enum Type
	 * @brief Server state
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
	ServerState(Type type);

	void prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd);

	inline Type type() const noexcept
	{
		return m_type;
	}
};

} // !irccd

#endif // !_IRCCD_SERVER_STATE_H_
