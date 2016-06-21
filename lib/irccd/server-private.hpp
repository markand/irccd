/*
 * server-private.hpp -- libircclient bridge
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

#ifndef IRCCD_SERVER_PRIVATE_HPP
#define IRCCD_SERVER_PRIVATE_HPP

#include <memory>

#include <libircclient.h>

#include "server.hpp"

namespace irccd {

/**
 * \brief Bridge for libircclient
 */
class Server::Session {
public:
    /**
     * libircclient handle.
     */
    using Handle = std::unique_ptr<irc_session_t, void (*)(irc_session_t *)>;

private:
    Handle m_handle;

public:
    /**
     * Create a null session.
     */
    inline Session()
        : m_handle(nullptr, nullptr)
    {
    }

    /**
     * Convert the libircclient session.
     */
    inline operator const irc_session_t *() const noexcept
    {
        return m_handle.get();
    }

    /**
     * Overloaded function.
     */
    inline operator irc_session_t *() noexcept
    {
        return m_handle.get();
    }

    /**
     * Get the handle.
     *
     * \return the handle
     */
    inline Handle &handle() noexcept
    {
        return m_handle;
    }
};

} // !irccd

#endif // !IRCCD_SERVER_PRIVATE_HPP
