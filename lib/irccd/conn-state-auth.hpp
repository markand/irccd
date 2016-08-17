/*
 * conn-state-auth.hpp -- connection is authenticating
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

#ifndef IRCCD_CONN_STATE_AUTH_HPP
#define IRCCD_CONN_STATE_AUTH_HPP

/**
 * \file conn-state-auth.hpp
 * \brief Connection is authenticating.
 */

#include "conn-state.hpp"

namespace irccd {

/**
 * \brief Authentication in progress.
 *
 * This state emit the authentication command and receives the response to see
 * if authentication succeeded.
 */
class Connection::AuthState : public Connection::State {
private:
    enum {
        Created,
        Sending,
        Checking
    } m_auth{Created};

    std::string m_output;

    void send(Connection &cnt) noexcept;
    void check(Connection &cnt) noexcept;

public:

    /**
     * \copydoc State::status
     */
    Status status() const noexcept override;

    /**
     * \copydoc State::prepare
     */
    void prepare(Connection &cnt, fd_set &in, fd_set &out) override;

    /**
     * \copydoc State::sync
     */
    void sync(Connection &cnt, fd_set &in, fd_set &out) override;
};

} // !irccd

#endif // !IRCCD_CONN_STATE_AUTH_HPP
