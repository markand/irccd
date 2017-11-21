/*
 * transport_server.cpp -- server side transports
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

#include "sysconfig.hpp"

#include <cassert>

#include "json_util.hpp"
#include "transport_server.hpp"

namespace irccd {

void transport_server::do_auth(std::shared_ptr<transport_client> client, accept_t handler)
{
    assert(client);
    assert(handler);

    client->recv([this, client, handler] (auto code, auto message) {
        if (code) {
            handler(std::move(code), std::move(client));
            return;
        }

        clients_.insert(client);

        auto command = json_util::to_string(message["command"]);
        auto password = json_util::to_string(message["password"]);

        if (command != "auth") {
            client->error(network_errc::auth_required);
            code = network_errc::auth_required;
        } else if (password != password_) {
            client->error(network_errc::invalid_auth);
            code = network_errc::invalid_auth;
        } else {
            client->set_state(transport_client::state_t::ready);
            code = network_errc::no_error;
        }

        handler(std::move(code), std::move(client));
    });
}

void transport_server::do_greetings(std::shared_ptr<transport_client> client, accept_t handler)
{
    assert(client);
    assert(handler);

    auto greetings = nlohmann::json({
        { "program",    "irccd"             },
        { "major",      IRCCD_VERSION_MAJOR },
        { "minor",      IRCCD_VERSION_MINOR },
        { "patch",      IRCCD_VERSION_PATCH },
#if defined(HAVE_JS)
        { "javascript", true                },
#endif
#if defined(HAVE_SSL)
        { "ssl",        true                },
#endif
    });

    client->send(greetings, [this, client, handler] (auto code) {
        if (code)
            handler(std::move(code), std::move(client));
        else if (!password_.empty())
            do_auth(std::move(client), std::move(handler));
        else {
            clients_.insert(client);
            handler(std::move(code), std::move(client));
        }
    });
}

void transport_server::accept(accept_t handler)
{
    assert(handler);

    do_accept([this, handler] (auto code, auto client) {
        if (code)
            handler(std::move(code), nullptr);
        else
            do_greetings(std::move(client), std::move(handler));
    });
}

#if defined(HAVE_SSL)

void tls_transport_server::do_handshake(std::shared_ptr<tls_transport_client> client, accept_t handler)
{
    client->socket().async_handshake(boost::asio::ssl::stream_base::server, [client, handler] (auto code) {
        if (code)
            handler(nullptr, code);
        else
            handler(std::move(client), std::move(code));
    });
}

tls_transport_server::tls_transport_server(acceptor_t acceptor, context_t context)
    : tcp_transport_server(std::move(acceptor))
    , context_(std::move(context))
{
}

void tls_transport_server::do_accept(accept_t handler)
{
    auto client = std::make_shared<tls_transport_client>(*this, acceptor_.get_io_service(), context_);

    acceptor_.async_accept(client->socket().lowest_layer(), [this, client, handler] (auto code) {
        if (code)
            handler(nullptr, code);
        else
            do_handshake(std::move(client), std::move(handler));
    });
}

#endif // !HAVE_SSL

} // !irccd