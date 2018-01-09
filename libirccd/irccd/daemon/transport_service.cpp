/*
 * transport_service.cpp -- transport service
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

#include <irccd/sysconfig.hpp>

#include <cassert>

#include <irccd/string_util.hpp>

#include "command_service.hpp"
#include "ip_transport_server.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "transport_client.hpp"
#include "transport_service.hpp"

#if !defined(IRCCD_SYSTEM_WINDOWS)
#   include "local_transport_server.hpp"
#endif

#if defined(HAVE_SSL)
#   include "tls_transport_server.hpp"
#endif

namespace irccd {

namespace {

std::unique_ptr<transport_server> load_transport_ip(boost::asio::io_service& service, const ini::section& sc)
{
    assert(sc.key() == "transport");

    std::unique_ptr<transport_server> transport;
    ini::section::const_iterator it;

    // Port.
    if ((it = sc.find("port")) == sc.cend())
        throw std::invalid_argument("missing 'port' parameter");

    auto port = string_util::to_uint<std::uint16_t>(it->value());

    // Address.
    std::string address = "*";

    if ((it = sc.find("address")) != sc.end())
        address = it->value();

    // 0011
    //    ^ define IPv4
    //   ^  define IPv6
    auto mode = 1U;

    /*
     * Documentation stated family but code checked for 'domain' option.
     *
     * As irccdctl uses domain, accept both and unify the option name to 'family'.
     *
     * See #637
     */
    if ((it = sc.find("domain")) != sc.end() || (it = sc.find("family")) != sc.end()) {
        mode = 0U;

        for (const auto& v : *it) {
            if (v == "ipv4")
                mode |= (1U << 0);
            if (v == "ipv6")
                mode |= (1U << 1);
        }
    }

    if (mode == 0U)
        throw std::invalid_argument("family must at least have ipv4 or ipv6");

    auto protocol = (mode & 0x2U)
        ? boost::asio::ip::tcp::v4()
        : boost::asio::ip::tcp::v6();

    // Optional SSL.
    std::string pkey;
    std::string cert;

    if ((it = sc.find("ssl")) != sc.end() && string_util::is_boolean(it->value())) {
        if ((it = sc.find("certificate")) == sc.end())
            throw std::invalid_argument("missing 'certificate' parameter");

        cert = it->value();

        if ((it = sc.find("key")) == sc.end())
            throw std::invalid_argument("missing 'key' parameter");

        pkey = it->value();
    }

    auto endpoint = (address == "*")
        ? boost::asio::ip::tcp::endpoint(protocol, port)
        : boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(address), port);

    boost::asio::ip::tcp::acceptor acceptor(service, endpoint, true);

    if (pkey.empty())
        return std::make_unique<ip_transport_server>(std::move(acceptor));

#if defined(HAVE_SSL)
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

    ctx.use_private_key_file(pkey, boost::asio::ssl::context::pem);
    ctx.use_certificate_file(cert, boost::asio::ssl::context::pem);

    return std::make_unique<tls_transport_server>(std::move(acceptor), std::move(ctx));
#else
    throw std::invalid_argument("SSL disabled");
#endif
}

std::unique_ptr<transport_server> load_transport_unix(boost::asio::io_service& service, const ini::section& sc)
{
    using boost::asio::local::stream_protocol;

    assert(sc.key() == "transport");

#if !defined(IRCCD_SYSTEM_WINDOWS)
    ini::section::const_iterator it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("missing 'path' parameter");

    // Remove the file first.
    std::remove(it->value().c_str());

    stream_protocol::endpoint endpoint(it->value());
    stream_protocol::acceptor acceptor(service, std::move(endpoint));

    return std::make_unique<local_transport_server>(std::move(acceptor));
#else
    (void)sc;

    throw std::invalid_argument("unix transports not supported on on this platform");
#endif
}

std::unique_ptr<transport_server> load_transport(boost::asio::io_service& service, const ini::section& sc)
{
    assert(sc.key() == "transport");

    std::unique_ptr<transport_server> transport;
    ini::section::const_iterator it = sc.find("type");

    if (it == sc.end())
        throw std::invalid_argument("missing 'type' parameter");

    if (it->value() == "ip")
        transport = load_transport_ip(service, sc);
    else if (it->value() == "unix")
        transport = load_transport_unix(service, sc);
    else
        throw std::invalid_argument(string_util::sprintf("invalid type given: %s", it->value()));

    if ((it = sc.find("password")) != sc.end())
        transport->set_password(it->value());

    return transport;
}

} // !namespace

void transport_service::handle_command(std::shared_ptr<transport_client> tc, const nlohmann::json& object)
{
    assert(object.is_object());

    auto name = object.find("command");

    if (name == object.end() || !name->is_string()) {
        tc->error(irccd_error::invalid_message);
        return;
    }

    auto cmd = irccd_.commands().find(*name);

    if (!cmd)
        tc->error(irccd_error::invalid_command, name->get<std::string>());
    else {
        try {
            cmd->exec(irccd_, *tc, object);
        } catch (const boost::system::system_error& ex) {
            tc->error(ex.code(), cmd->get_name());
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "transport: unknown error not reported" << std::endl;
            irccd_.log().warning() << "transport: " << ex.what() << std::endl;
        }
    }
}

void transport_service::do_recv(std::shared_ptr<transport_client> tc)
{
    tc->recv([this, tc] (auto code, auto json) {
        switch (code.value()) {
        case boost::system::errc::network_down:
            irccd_.log().warning("transport: client disconnected");
            break;
            case boost::system::errc::invalid_argument:
            tc->error(irccd_error::invalid_message);
            break;
        default:
            handle_command(tc, json);

            if (tc->state() == transport_client::state_t::ready)
                do_recv(std::move(tc));

            break;
        }
    });
}

void transport_service::do_accept(transport_server& ts)
{
    ts.accept([this, &ts] (auto code, auto client) {
        if (code)
            irccd_.log().warning() << "transport: new client error: " << code.message() << std::endl;
        else {
            do_accept(ts);
            do_recv(std::move(client));

            irccd_.log().info() << "transport: new client connected" << std::endl;
        }
    });
}

transport_service::transport_service(irccd& irccd) noexcept
    : irccd_(irccd)
{
}

transport_service::~transport_service() noexcept = default;

void transport_service::add(std::unique_ptr<transport_server> ts)
{
    assert(ts);

    do_accept(*ts);
    servers_.push_back(std::move(ts));
}

void transport_service::broadcast(const nlohmann::json& json)
{
    assert(json.is_object());

    for (const auto& servers : servers_)
        for (const auto& client : servers->clients())
            client->send(json);
}

void transport_service::load(const config& cfg) noexcept
{
    for (const auto& section : cfg.doc()) {
        if (section.key() != "transport")
            continue;

        try {
            add(load_transport(irccd_.service(), section));
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "transport: " << ex.what() << std::endl;
        }
    }
}

} // !irccd
