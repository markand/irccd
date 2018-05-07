/*
 * command.hpp -- remote command
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

#ifndef IRCCD_DAEMON_COMMAND_HPP
#define IRCCD_DAEMON_COMMAND_HPP

/**
 * \file command.hpp
 * \brief Remote commands.
 */

#include <string>

#include <irccd/sysconfig.hpp>
#include <irccd/json_util.hpp>

namespace irccd {

class irccd;
class irccdctl;
class transport_client;

/**
 * \brief Server side remote command
 */
class command {
public:
    /**
     * Convenient alias.
     */
    using document = json_util::document;

    /**
     * Default destructor virtual.
     */
    virtual ~command() = default;

    /**
     * Return the command name, must not have spaces.
     *
     * \return the command name
     */
    virtual std::string get_name() const noexcept = 0;

    /**
     * Execute the command.
     *
     * If the command throw an exception, the error is sent to the client so be
     * careful about sensitive information.
     *
     * The implementation should use client.success() or client.error() to send
     * some data.
     *
     * \param irccd the irccd instance
     * \param client the client
     * \param args the client arguments
     */
    virtual void exec(irccd& irccd, transport_client& client, const document& args) = 0;
};

} // !irccd

#endif // !IRCCD_DAEMON_COMMAND_HPP
