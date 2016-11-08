/*
 * cli-server-join.hpp -- implementation of irccdctl server-join
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

#ifndef IRCCDCTL_CLI_SERVER_JOIN_HPP
#define IRCCDCTL_CLI_SERVER_JOIN_HPP

/**
 * \file cli-server-join.hpp
 * \brief Implementation of irccdctl server-join.
 */

#include "cli.hpp"

namespace irccd {

namespace cli {

/**
 * \brief Implementation of irccdctl server-join.
 */
class ServerJoinCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerJoinCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

} // !command

} // !irccd

#endif // !IRCCDCTL_CLI_SERVER_JOIN_HPP
