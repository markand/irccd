/*
 * conn-state-checking.hpp -- verify irccd instance
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

#ifndef IRCCD_CONN_STATE_CHECKING_HPP
#define IRCCD_CONN_STATE_CHECKING_HPP

/**
 * \file conn-state-checking.hpp
 * \brief Verify irccd instance and version
 */

#include "conn-state.hpp"

namespace irccd {

/**
 * \brief State for veryfing connection.
 *
 * This state is used when socket connection is complete but we have not
 * verified that the endpoint is an irccd instance.
 *
 * This state also verifies that the irccd daemon is compatible with
 * our library.
 */
class Connection::CheckingState : public Connection::State {
private:
    void verifyProgram(const nlohmann::json &json) const;
    void verifyVersion(Connection &cnx, const nlohmann::json &json) const;
    void verify(Connection &cnx) const;

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

#endif // !IRCCD_CONN_STATE_CHECKING_HPP
