/*
 * command.hpp -- remote command
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

#ifndef IRCCD_COMMAND_HPP
#define IRCCD_COMMAND_HPP

/**
 * \file command.hpp
 * \brief Remote commands.
 */

#include <cassert>
#include <map>
#include <vector>

#include "json.hpp"
#include "sysconfig.hpp"

namespace irccd {

class Irccd;
class Irccdctl;
class TransportClient;

/**
 * \brief Server side remote command
 */
class Command {
private:
    std::string m_name;

public:
    /**
     * Construct a command.
     *
     * \pre !name.empty()
     * \param name the command name
     */
    inline Command(std::string name) noexcept
        : m_name(std::move(name))
    {
        assert(!m_name.empty());
    }

    /**
     * Default destructor virtual.
     */
    virtual ~Command() = default;

    /**
     * Return the command name, must not have spaces.
     *
     * \return the command name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }

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
    IRCCD_EXPORT virtual void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) = 0;
};

} // !irccd

#endif // !IRCCD_COMMAND_HPP
