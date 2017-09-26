/*
 * transport.cpp -- irccd transports
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
#include <cstdio>

#include "transport.hpp"

namespace irccd {

/*
 * transport_client
 * ------------------------------------------------------------------
 */

void transport_client::error(const std::string& msg)
{
    state_ = state::closing;

    send({{ "error", msg }});
}

void transport_client::flush() noexcept
{
    for (std::size_t pos; (pos = input_.find("\r\n\r\n")) != std::string::npos; ) {
        auto message = input_.substr(0, pos);

        input_.erase(input_.begin(), input_.begin() + pos + 4);

        try {
            auto document = nlohmann::json::parse(message);

            if (!document.is_object())
                error("invalid argument");
            else
                on_command(document);
        } catch (const std::exception& ex) {
            error(ex.what());
        }
    }
}

void transport_client::authenticate() noexcept
{
    auto pos = input_.find("\r\n\r\n");

    if (pos == std::string::npos)
        return;

    auto msg = input_.substr(0, pos);

    input_.erase(input_.begin(), input_.begin() + pos + 4);

    try {
        auto doc = nlohmann::json::parse(msg);

        if (!doc.is_object())
            error("invalid argument");

        auto cmd = doc.find("command");

        if (cmd == doc.end() || !cmd->is_string() || *cmd != "auth")
            error("authentication required");

        auto pw = doc.find("password");
        auto result = true;

        if (pw == doc.end() || !pw->is_string() || *pw != parent_.password()) {
            state_ = state::closing;
            result = false;
        } else
            state_ = state::ready;

        send({
            { "response", "auth" },
            { "result", result }
        });
    } catch (const std::exception& ex) {
        error(ex.what());
    }
}

void transport_client::recv() noexcept
{
    try {
        std::string buffer;

        buffer.resize(512);
        buffer.resize(recv(&buffer[0], buffer.size()));

        if (buffer.empty())
            on_die();

        input_ += std::move(buffer);
    } catch (const std::exception &) {
        on_die();
    }
}

void transport_client::send() noexcept
{
    try {
        auto ns = send(&output_[0], output_.size());

        if (ns == 0)
            on_die();

        output_.erase(0, ns);
    } catch (const std::exception&) {
        on_die();
    }
}

unsigned transport_client::recv(void* buffer, unsigned length)
{
    return socket_.recv(buffer, length);
}

unsigned transport_client::send(const void* buffer, unsigned length)
{
    return socket_.send(buffer, length);
}

transport_client::transport_client(transport_server& parent, net::TcpSocket socket)
    : parent_(parent)
    , socket_(std::move(socket))
{
    assert(socket_.isOpen());

    socket_.set(net::option::SockBlockMode(false));

    // Send some information.
    auto object = nlohmann::json::object({
        { "program",    "irccd"                 },
        { "major",      IRCCD_VERSION_MAJOR     },
        { "minor",      IRCCD_VERSION_MINOR     },
        { "patch",      IRCCD_VERSION_PATCH     }
    });

#if defined(WITH_JS)
    object.push_back({"javascript", true});
#endif
#if defined(WITH_SSL)
    object.push_back({"ssl", true});
#endif

    send(object);
}

void transport_client::prepare(fd_set& in, fd_set& out, net::Handle& max)
{
    if (socket_.handle() > max)
        max = socket_.handle();

    switch (state_) {
    case state::greeting:
        FD_SET(socket_.handle(), &in);
        FD_SET(socket_.handle(), &out);
        break;
    case state::authenticating:
        FD_SET(socket_.handle(), &in);
        break;
    case state::ready:
        FD_SET(socket_.handle(), &in);

        if (!output_.empty())
            FD_SET(socket_.handle(), &out);
        break;
    case state::closing:
        if (!output_.empty())
            FD_SET(socket_.handle(), &out);
        else
            on_die();
        break;
    default:
        break;
    }
}

void transport_client::sync(fd_set& in, fd_set& out)
{
    switch (state_) {
    case state::greeting:
        if (FD_ISSET(socket_.handle(), &in))
            recv();
        else if (FD_ISSET(socket_.handle(), &out))
            send();

        if (output_.empty())
            state_ = parent_.password().empty() ? state::ready : state::authenticating;

        break;
    case state::authenticating:
        if (FD_ISSET(socket_.handle(), &in))
            recv();

        authenticate();
        break;
    case state::ready:
        if (FD_ISSET(socket_.handle(), &in))
            recv();
        if (FD_ISSET(socket_.handle(), &out))
            send();

        flush();
        break;
    case state::closing:
        if (FD_ISSET(socket_.handle(), &out))
            send();
        break;
    default:
        break;
    }
}

void transport_client::send(const nlohmann::json& json)
{
    assert(json.is_object());

    output_ += json.dump();
    output_ += "\r\n\r\n";
}

void transport_client::success(const std::string& cmd, nlohmann::json extra)
{
    assert(extra.is_object() || extra.is_null());

    if (!extra.is_object())
        extra = nlohmann::json::object();

    extra["command"] = cmd;
    extra["status"] = true;

    output_ += extra.dump();
    output_ += "\r\n\r\n";
}

void transport_client::error(const std::string& cmd, const std::string& error, nlohmann::json extra)
{
    assert(extra.is_object() || extra.is_null());

    if (!extra.is_object())
        extra = nlohmann::json::object();

    extra["command"] = cmd;
    extra["status"] = false;
    extra["error"] = error;

    output_ += extra.dump();
    output_ += "\r\n\r\n";
}

/*
 * transport_client_tls
 * ------------------------------------------------------------------
 */

#if defined(WITH_SSL)

void transport_client_tls::handshake()
{
    try {
        ssl_.handshake();
        handshake_ = handshake::ready;
    } catch (const net::WantReadError&) {
        handshake_ = handshake::read;
    } catch (const net::WantWriteError&) {
        handshake_ = handshake::write;
    } catch (const std::exception&) {
        on_die();
    }
}

transport_client_tls::transport_client_tls(const std::string& pkey,
                                           const std::string& cert,
                                           transport_server& parent,
                                           net::TcpSocket socket)
    : transport_client(parent, std::move(socket))
    , ssl_(socket_)
{
    ssl_.setPrivateKey(pkey);
    ssl_.setCertificate(cert);

    handshake();
}

unsigned transport_client_tls::recv(void* buffer, unsigned length)
{
    unsigned nread = 0;

    try {
        nread = ssl_.recv(buffer, length);
    } catch (const net::WantReadError&) {
        handshake_ = handshake::read;
    } catch (const net::WantWriteError&) {
        handshake_ = handshake::write;
    } catch (const std::exception&) {
        on_die();
    }

    return nread;
}

unsigned transport_client_tls::send(const void* buffer, unsigned length)
{
    unsigned nsent = 0;

    try {
        nsent = ssl_.send(buffer, length);
    } catch (const net::WantReadError&) {
        handshake_ = handshake::read;
    } catch (const net::WantWriteError &) {
        handshake_ = handshake::write;
    } catch (const std::exception&) {
        on_die();
    }

    return nsent;
}

void transport_client_tls::prepare(fd_set& in, fd_set& out, net::Handle& max)
{
    if (socket_.handle() > max)
        max = socket_.handle();

    switch (handshake_) {
    case handshake::read:
        FD_SET(socket_.handle(), &in);
        break;
    case handshake::write:
        FD_SET(socket_.handle(), &out);
        break;
    default:
        transport_client::prepare(in, out, max);
        break;
    }
}

void transport_client_tls::sync(fd_set& in, fd_set& out)
{
    switch (handshake_) {
    case handshake::read:
    case handshake::write:
        handshake();
        break;
    default:
        transport_client::sync(in, out);
    }
}

#endif  // !WITH_SSL

/*
 * transport_server_ip
 * ------------------------------------------------------------------
 */

transport_server_ip::transport_server_ip(const std::string& address,
                                         std::uint16_t port,
                                         std::uint8_t mode)
    : transport_server(net::TcpSocket((mode & v6) ? AF_INET6 : AF_INET, 0))
{
    assert((mode & v6) || (mode & v4));

    socket_.set(net::option::SockReuseAddress(true));

    if (mode & v6) {
        // Disable or enable IPv4 when using IPv6.
        socket_.set(net::option::Ipv6Only(!(mode & v4)));

        if (address == "*")
            socket_.bind(net::ipv6::any(port));
        else
            socket_.bind(net::ipv6::pton(address, port));
    } else {
        if (address == "*")
            socket_.bind(net::ipv4::any(port));
        else
            socket_.bind(net::ipv4::pton(address, port));
    }

    socket_.listen();
}

std::uint16_t transport_server_ip::port() const
{
    auto addr = socket_.getsockname();

    return addr.domain() == AF_INET
        ? ntohs(addr.as<sockaddr_in>().sin_port)
        : ntohs(addr.as<sockaddr_in6>().sin6_port);
}

/*
 * transport_server_tls
 * ------------------------------------------------------------------
 */

#if defined(WITH_SSL)

transport_server_tls::transport_server_tls(const std::string& pkey,
                                           const std::string& cert,
                                           const std::string& address,
                                           std::uint16_t port,
                                           std::uint8_t mode)
    : transport_server_ip(address, port, mode)
    , privatekey_(pkey)
    , cert_(cert)
{
}

std::unique_ptr<transport_client> transport_server_tls::accept()
{
    return std::make_unique<transport_client_tls>(privatekey_, cert_, *this, socket_.accept());
}

#endif  // !WITH_SSL

/*
 * transport_server_local
 * ------------------------------------------------------------------
 */

#if !defined(IRCCD_SYSTEM_WINDOWS)

transport_server_local::transport_server_local(std::string path)
    : transport_server(net::TcpSocket(AF_LOCAL, 0))
    , path_(std::move(path))
{
    socket_.bind(net::local::create(path_, true));
    socket_.listen();
}

transport_server_local::~transport_server_local()
{
    ::remove(path_.c_str());
}

#endif

} // !irccd
