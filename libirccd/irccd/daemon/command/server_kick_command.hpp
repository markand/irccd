/*
 * server_kick_command.hpp -- implementation of server-kick transport command
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_SERVER_KICK_COMMAND_HPP
#define IRCCD_DAEMON_SERVER_KICK_COMMAND_HPP

/**
 * \file server_kick_command.hpp
 * \brief Implementation of server-kick transport command.
 */

#include <irccd/daemon/command.hpp>

namespace irccd {

/**
 * \brief Implementation of server-kick transport command.
 *
 * Replies:
 *
 *   - server_error::invalid_channel,
 *   - server_error::invalid_identifier,
 *   - server_error::invalid_nickname,
 *   - server_error::not_found.
 */
class server_kick_command : public command {
public:
    /**
     * \copydoc command::get_name
     */
    std::string get_name() const noexcept override;

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

} // !irccd

#endif // !IRCCD_DAEMON_SERVER_KICK_COMMAND_HPP
