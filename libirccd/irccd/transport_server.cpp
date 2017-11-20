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

#include "transport_server.hpp"

namespace irccd {

bool transport_server::do_auth_check(nlohmann::json message, accept_t handler)
{
    assert(handler);

    auto command = message["command"];
    auto password = message["password"];

    if (!command.is_string() || !password.is_string()) {
        handler(nullptr, network_error::invalid_message);
        return false;
    }

    if (command != "auth" || password.get<std::string>() != password_) {
        handler(nullptr, network_error::invalid_auth);
        return false;
    }

    return true;
}

void transport_server::do_auth(std::shared_ptr<transport_client> client, accept_t handler)
{
    assert(client);
    assert(handler);

    client->recv([this, client, handler] (auto message, auto code) {
        if (code)
            handler(client, code);
        if (do_auth_check(message, handler)) {
            clients_.insert(client);
            client->set_state(transport_client::state_t::ready);
            handler(client, code);
        }
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
            handler(client, code);
        else if (!password_.empty())
            do_auth(std::move(client), std::move(handler));
        else {
            clients_.insert(client);
            handler(client, code);
        }
    });
}

void transport_server::accept(accept_t handler)
{
    assert(handler);

    do_accept([this, handler] (auto client, auto code) {
        if (code)
            handler(nullptr, code);
        else
            do_greetings(std::move(client), std::move(handler));
    });
}

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

} // !irccd
