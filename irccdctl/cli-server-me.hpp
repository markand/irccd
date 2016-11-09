/*
 * cli-server-me.hpp -- implementation of irccdctl server-me
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

#ifndef IRCCDCTL_CLI_SERVER_ME_HPP
#define IRCCDCTL_CLI_SERVER_ME_HPP

/**
 * \file cli-server-me.hpp
 * \brief Implementation of irccdctl server-me.
 */

#include "cli.hpp"

namespace irccd {

namespace cli {

/**
 * \brief Implementation of irccdctl server-me.
 */
class ServerMeCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerMeCli();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

} // !cli

} // !irccd

#endif // !_IRCCDCTL_COMMAND_SERVER_ME_H_
