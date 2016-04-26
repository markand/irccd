/*
 * server-state-connecting.hpp -- connecting state
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

#ifndef IRCCD_SERVER_STATE_CONNECTING_HPP
#define IRCCD_SERVER_STATE_CONNECTING_HPP

/**
 * \file server-state-connecting.hpp
 * \brief Connecting state.
 */

#include "elapsed-timer.hpp"
#include "server-state.hpp"

namespace irccd {

namespace state {

/**
 * \brief Connecting state.
 */
class Connecting : public ServerState {
private:
	bool m_started{false};
	ElapsedTimer m_timer;

public:
	/**
	 * \copydoc ServerState::prepare
	 */
	void prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd) override;

	/**
	 * \copydoc ServerState::ident
	 */
	std::string ident() const override;
};

} // !state

} // !irccd

#endif // !IRCCD_SERVER_STATE_CONNECTING_HPP