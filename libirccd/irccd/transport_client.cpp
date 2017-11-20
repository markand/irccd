/*
 * transport_client.cpp -- server side transport clients
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

#include <cassert>

#include "transport_client.hpp"
#include "transport_server.hpp"

namespace irccd {

void transport_client::close()
{
    state_ = state_t::closing;
    output_.clear();
    parent_.clients().erase(shared_from_this());
}

void transport_client::flush()
{
    if (output_.empty())
        return;

    auto self = shared_from_this();
    auto size = output_[0].first.size();

    do_send(output_[0].first, [this, self, size] (auto code, auto xfer) {
        if (output_[0].second)
            output_[0].second(code);

        output_.pop_front();

        if (code || xfer != size || (output_.empty() && state_ == state_t::closing))
            close();
        else if (!output_.empty())
            flush();
    });
}

void transport_client::recv(recv_t handler)
{
    assert(handler);

    auto self = shared_from_this();

    do_recv(input_, [this, self, handler] (auto code, auto xfer) {
        if (code || xfer == 0) {
            handler("", code);
            close();
            return;
        }

        std::string message(
            boost::asio::buffers_begin(input_.data()),
            boost::asio::buffers_begin(input_.data()) + xfer - 4
        );

        // Remove early in case of errors.
        input_.consume(xfer);

        try {
            auto json = nlohmann::json::parse(message);

            if (!json.is_object())
                handler(nullptr, network_error::invalid_message);
            else
                handler(json, code);
        } catch (...) {
            handler(nullptr, network_error::invalid_message);
        }
    });
}

void transport_client::send(const nlohmann::json& data, send_t handler)
{
    assert(data.is_object());

    if (state_ == state_t::closing)
        return;

    auto in_progress = !output_.empty();

    output_.emplace_back(data.dump() + "\r\n\r\n", std::move(handler));

    if (!in_progress)
        flush();
}

void transport_client::error(const nlohmann::json& data, send_t handler)
{
    send(std::move(data), std::move(handler));
    set_state(state_t::closing);
}

} // !irccd
