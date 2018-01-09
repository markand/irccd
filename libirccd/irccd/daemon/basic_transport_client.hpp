/*
 * basic_transport_client.hpp -- simple socket transport client
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

#ifndef IRCCD_BASIC_TRANSPORT_CLIENT_HPP
#define IRCCD_BASIC_TRANSPORT_CLIENT_HPP

#include <irccd/network_stream.hpp>

#include "transport_client.hpp"

namespace irccd {

/**
 * \brief Transport client for sockets.
 *
 * This class implements do_recv/do_send for Boost.Asio based socket streams.
 */
template <typename Socket>
class basic_transport_client : public transport_client {
private:
    network_stream<Socket> stream_;

public:
    /**
     * Construct the client.
     *
     * \param parent the parent
     * \param args the argument to pass to the network_stream
     */
    template <typename... Args>
    inline basic_transport_client(transport_server& parent, Args&&... args)
        : transport_client(parent)
        , stream_(std::forward<Args>(args)...)
    {
    }

    /**
     * Get the underlying stream.
     *
     * \return the stream
     */
    inline const network_stream<Socket>& stream() const noexcept
    {
        return stream_;
    }

    /**
     * Overloaded function.
     *
     * \return the stream
     */
    inline network_stream<Socket>& stream() noexcept
    {
        return stream_;
    }

    /**
     * \copydoc transport_client::do_recv
     */
    void do_recv(network_recv_handler handler) override
    {
        assert(handler);

        auto self = shared_from_this();

        stream_.recv([this, self, handler] (auto msg, auto code) {
            handler(std::move(msg), std::move(code));
        });
    }

    /**
     * \copydoc transport_client::do_send
     */
    void do_send(nlohmann::json json, network_send_handler handler) override
    {
        auto self = shared_from_this();

        stream_.send(std::move(json), [this, self, handler] (auto code) {
            if (handler)
                handler(std::move(code));
        });
    }
};

} // !irccd

#endif // !IRCCD_BASIC_TRANSPORT_CLIENT_HPP
