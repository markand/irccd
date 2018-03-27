/*
 * transport_client.cpp -- server side transport clients
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

#include <cassert>

#include "transport_client.hpp"
#include "transport_server.hpp"

namespace irccd {

void transport_client::recv(network_recv_handler handler)
{
    if (state_ != state_t::closing)
        do_recv(std::move(handler));
}

void transport_client::send(nlohmann::json json, network_send_handler handler)
{
    if (state_ != state_t::closing)
        do_send(std::move(json), std::move(handler));
}

void transport_client::success(const std::string& cname, network_send_handler handler)
{
    assert(!cname.empty());

    send({{ "command", cname }}, std::move(handler));
}

void transport_client::error(boost::system::error_code code, network_send_handler handler)
{
    error(std::move(code), "", std::move(handler));
}

void transport_client::error(boost::system::error_code code,
                             std::string cname,
                             network_send_handler handler)
{
    assert(code);

    auto json = nlohmann::json::object({
        { "error",          code.value()            },
        { "errorCategory",  code.category().name()  },
        { "errorMessage",   code.message()          }
    });

    if (!cname.empty())
        json["command"] = std::move(cname);

    send(std::move(json), [this, handler] (auto code) {
        if (handler)
            handler(code);

        parent_.get_clients().erase(shared_from_this());
    });

    state_ = state_t::closing;
}

} // !irccd
