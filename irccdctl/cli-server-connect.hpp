/*
 * command-server-connect.h -- implementation of irccdctl server-connect
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

#ifndef IRCCDCTL_CLI_SERVER_CONNECT_HPP
#define IRCCDCTL_CLI_SERVER_CONNECT_HPP

/**
 * \file cli-server-connect.hpp
 * \brief Implementation of irccdctl server-connect.
 */

#include "cli.hpp"

namespace irccd {

namespace cli {

/**
 * \brief Implementation of irccdctl server-connect.
 */
class ServerConnectCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerConnectCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

} // !cli

} // !irccd

#endif // !IRCCDCTL_CLI_SERVER_CONNECT_HPP
