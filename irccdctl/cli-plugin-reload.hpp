/*
 * cli-plugin-reload.hpp -- implementation of irccdctl plugin-reload
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

#ifndef IRCCDCTL_CLI_PLUGIN_RELOAD_HPP
#define IRCCDCTL_CLI_PLUGIN_RELOAD_HPP

/**
 * \file cli-plugin-reload.hpp
 * \brief Implementation of irccdctl plugin-reload.
 */

#include "cli.hpp"

namespace irccd {

namespace cli {

/**
 * \brief Implementation of irccdctl plugin-reload.
 */
class PluginReloadCli : public Cli {
public:
    PluginReloadCli();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

} // !cli

} // !irccd

#endif // !IRCCDCTL_CLI_PLUGIN_RELOAD_HPP
