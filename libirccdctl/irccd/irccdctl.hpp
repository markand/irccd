/*
 * irccdctl.hpp -- main irccdctl class
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

#ifndef IRCCD_IRCCDCTL_HPP
#define IRCCD_IRCCDCTL_HPP

/**
 * \file irccdctl.hpp
 * \brief Main irccdctl class
 */

#include <memory>

#include "client.hpp"

namespace irccd {

/**
 * \brief Transient class for connecting to irccd
 */
class Irccdctl {
private:
    std::unique_ptr<Client> m_client;

public:
    /**
     * Create the irccdctl instance with the specified client.
     *
     * \param client the client
     */
    inline Irccdctl(std::unique_ptr<Client> client) noexcept
        : m_client(std::move(client))
    {
    }

    /**
     * Get the client.
     *
     * \return the client reference
     */
    inline Client &client() noexcept
    {
        return *m_client;
    }

    /**
     * Overloaded function.
     *
     * \return the client
     */
    inline const Client &client() const noexcept
    {
        return *m_client;
    }

    /**
     * Pollable prepare function.
     *
     * \param in the input set
     * \param out the output set
     * \param max the maximum handle
     */
    inline void prepare(fd_set &in, fd_set &out, net::Handle &max)
    {
        m_client->prepare(in, out, max);
    }

    /**
     * Pollable sync function.
     *
     * \param in the input set
     * \param out the output set
     */
    inline void sync(fd_set &in, fd_set &out)
    {
        m_client->sync(in, out);
    }
};

} // !irccd

#endif // !IRCCD_IRCCDCTL_HPP
