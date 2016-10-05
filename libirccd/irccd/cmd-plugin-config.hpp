/*
 * cmd-plugin-config.hpp -- implementation of plugin-config command
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

#ifndef IRCCD_CMD_PLUGIN_CONFIG_HPP
#define IRCCD_CMD_PLUGIN_CONFIG_HPP

/**
 * \file cmd-plugin-config.hpp
 * \brief Implementation of plugin-config transport command.
 */

#include "command.hpp"

namespace irccd {

namespace command {

/**
 * \brief Implementation of plugin-config transport command.
 */
class PluginConfigCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginConfigCommand();

    /**
     * \copydoc Command::args
     */
    IRCCD_EXPORT std::vector<Arg> args() const override;

    /**
     * \copydoc Command::properties
     */
    IRCCD_EXPORT std::vector<Property> properties() const override;

    /**
     * \copydoc Command::request
     */
    IRCCD_EXPORT nlohmann::json request(Irccdctl &irccdctl, const CommandRequest &args) const override;

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT nlohmann::json exec(Irccd &irccd, const nlohmann::json &request) const override;

    /**
     * \copydoc Command::result
     */
    IRCCD_EXPORT void result(Irccdctl &irccdctl, const nlohmann::json &response) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_PLUGIN_CONFIG_HPP
