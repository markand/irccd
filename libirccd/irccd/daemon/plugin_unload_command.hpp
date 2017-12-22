/*
 * plugin_unload_command.hpp -- implementation of plugin-unload transport command
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_PLUGIN_UNLOAD_COMMAND_HPP
#define IRCCD_DAEMON_PLUGIN_UNLOAD_COMMAND_HPP

/**
 * \file plugin_unload_command.hpp
 * \brief Implementation of plugin-unload transport command.
 */

#include "command.hpp"

namespace irccd {

/**
 * \brief Implementation of plugin-unload transport command.
 *
 * Replies:
 *
 *   - plugin_error::not_found
 *   - plugin_error::exec_error
 */
class plugin_unload_command : public command {
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

#endif // !IRCCD_DAEMON_PLUGIN_UNLOAD_COMMAND_HPP
