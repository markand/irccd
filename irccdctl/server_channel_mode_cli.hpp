/*
 * server_channel_mode_cli.hpp -- implementation of irccdctl server-cmode
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

#ifndef IRCCD_CTL_SERVER_CHANNEL_MODE_CLI_HPP
#define IRCCD_CTL_SERVER_CHANNEL_MODE_CLI_HPP

/**
 * \file server_channel_mode_cli.hpp
 * \brief Implementation of irccdctl server-cmode.
 */

#include "cli.hpp"

namespace irccd {

namespace ctl {

/**
 * \brief Implementation of irccdctl server-cmode.
 */
class server_channel_mode_cli : public cli {
public:
    /**
     * \copydoc cli::name
     */
    std::string name() const override;

    /**
     * \copydoc cli::exec
     */
    void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

} // !ctl

} // !irccd

#endif // !IRCCD_CTL_SERVER_CHANNEL_MODE_CLI_HPP
