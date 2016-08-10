/*
 * conn-state.hpp -- abstract state for Connection object
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

#ifndef IRCCD_CONN_STATE_HPP
#define IRCCD_CONN_STATE_HPP

/**
 * \file conn-state.hpp
 * \brief State for Connection.
 */

#include "connection.hpp"

namespace irccd {

/**
 * \brief Abstract state interface for Connection
 *
 * Abstract state interface for Connection.
 *
 * The Connection is event based, you should not throw exceptions from the
 * prepare or sync functions, instead you should change the Connection state
 * and emit the onDisconnect signal.
 */
class Connection::State {
public:
    /**
     * Return the state.
     *
     * \return the state
     */
    virtual Status status() const noexcept = 0;

    /**
     * Prepare the input and output sets.
     *
     * You should not change the connection state in this function.
     *
     * \param cnt the connection object
     * \param in the input set
     * \param out the output set
     */
    virtual void prepare(Connection &cnt, fd_set &in, fd_set &out) = 0;

    /**
     * Synchronize network I/O in the implementation.
     *
     * You should change the connection state using cnx.m_nextState
     * if needed.
     *
     * \param cnt the connection object
     * \param in the input set
     * \param out the output set
     */
    virtual void sync(Connection &cnt, fd_set &in, fd_set &out) = 0;
};

} // !irccd

#endif // !IRCCD_CONN_STATE_HPP
