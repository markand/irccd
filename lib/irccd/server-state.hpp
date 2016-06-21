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

/**
 * \defgroup states Server states
 * \brief States for Server class.
 */

#include "elapsed-timer.hpp"
#include "net.hpp"
#include "sysconfig.hpp"

namespace irccd {

class Server;

/**
 * \brief Namespace for server states.
 */
namespace state {
}

/**
 * \class ServerState
 * \brief Server current state.
 */
class ServerState {
public:
    /**
     * Default constructor.
     */
    ServerState() = default;

    /**
     * Virtual default destructor.
     */
    virtual ~ServerState() = default;

    /**
     * Prepare the state.
     *
     * \param server the server
     * \param setinput the read set
     * \param setoutput the write set
     * \param maxfd the maximum fd
     */
    virtual void prepare(Server &server, fd_set &setinput, fd_set &setoutput, net::Handle &maxfd) = 0;

    /**
     * Return the state identifier, only for information purposes.
     *
     * \return the identifier
     */
    virtual std::string ident() const = 0;
};

} // !irccd

#endif // !IRCCD_SERVER_STATE_HPP
