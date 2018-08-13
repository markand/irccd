/*
 * transport_util.cpp -- transport utilities
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

#include <boost/predef/os.h>

#include <irccd/ini_util.hpp>
#include <irccd/string_util.hpp>
#include <irccd/socket_acceptor.hpp>

#if defined(IRCCD_HAVE_SSL)
#   include <irccd/tls_acceptor.hpp>
#endif

#include "transport_util.hpp"
#include "transport_server.hpp"

namespace asio = boost::asio;

namespace irccd::transport_util {

namespace {

auto from_config_load_ip_protocol(const ini::section& sc) -> asio::ip::tcp
{
    bool ipv4 = true, ipv6 = false;

    /*
     * Documentation stated family but code checked for 'domain' option.
     *
     * As irccdctl uses domain, accept both and unify the option name to 'family'.
     *
     * See #637
     */
    ini::section::const_iterator it;

    if ((it = sc.find("domain")) != sc.end() || (it = sc.find("family")) != sc.end()) {
        ipv4 = ipv6 = false;

        for (const auto& v : *it) {
            if (v == "ipv4")
                ipv4 = true;
            if (v == "ipv6")
                ipv6 = true;
        }
    }

    if (!ipv4 && !ipv6)
        throw transport_error(transport_error::invalid_family);

    return ipv4 ? asio::ip::tcp::v4() : asio::ip::tcp::v6();
}

auto from_config_load_ip_endpoint(const ini::section& sc) -> asio::ip::tcp::endpoint
{
    const auto port = ini_util::get_uint<std::uint16_t>(sc, "port");
    const auto address = ini_util::optional_string(sc, "address", "*");

    if (!port)
        throw transport_error(transport_error::invalid_port);
    if (address.empty())
        throw transport_error(transport_error::invalid_address);

    const auto protocol = from_config_load_ip_protocol(sc);

    return address == "*"
        ? asio::ip::tcp::endpoint(protocol, *port)
        : asio::ip::tcp::endpoint(asio::ip::address::from_string(address), *port);
}

auto from_config_load_ip_acceptor(asio::io_service& service, const ini::section& sc) -> asio::ip::tcp::acceptor
{
    return asio::ip::tcp::acceptor(service, from_config_load_ip_endpoint(sc), true);
}

auto from_config_load_ip(asio::io_service& service, const ini::section& sc) -> std::unique_ptr<transport_server>
{
    assert(sc.key() == "transport");

    auto acceptor = from_config_load_ip_acceptor(service, sc);

    if (string_util::is_boolean(sc.get("ssl").value())) {
#if !defined(IRCCD_HAVE_SSL)
        throw transport_error(transport_error::ssl_disabled);
#else
        const auto key = sc.get("key").value();
        const auto cert = sc.get("certificate").value();

        if (key.empty())
            throw transport_error(transport_error::invalid_private_key);
        if (cert.empty())
            throw transport_error(transport_error::invalid_certificate);

        boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

        ctx.use_private_key_file(key, boost::asio::ssl::context::pem);
        ctx.use_certificate_file(cert, boost::asio::ssl::context::pem);

        return std::make_unique<transport_server>(
            std::make_unique<io::tls_acceptor<>>(std::move(ctx), std::move(acceptor)));
#endif
    }

    return std::make_unique<transport_server>(
        std::make_unique<io::ip_acceptor>(std::move(acceptor)));
}

auto from_config_load_unix(asio::io_service& service, const ini::section& sc) -> std::unique_ptr<transport_server>
{
    assert(sc.key() == "transport");

#if !BOOST_OS_WINDOWS
    using boost::asio::local::stream_protocol;

    const auto path = sc.get("path").value();

    if (path.empty())
        throw transport_error(transport_error::invalid_path);

    // Remove the file first.
    std::remove(path.c_str());

    stream_protocol::endpoint endpoint(path);
    stream_protocol::acceptor acceptor(service, std::move(endpoint));

    return std::make_unique<transport_server>(
        std::make_unique<io::local_acceptor>(std::move(acceptor)));
#else
    (void)service;
    (void)sc;

    throw transport_error(transport_error::not_supported);
#endif
}

} // !namespace

auto from_config(asio::io_service& service, const ini::section& sc) -> std::unique_ptr<transport_server>
{
    assert(sc.key() == "transport");

    const auto type = sc.get("type").value();
    const auto password = sc.get("password").value();

    if (type.empty())
        throw transport_error(transport_error::not_supported);

    std::unique_ptr<transport_server> transport;

    if (type == "ip")
        transport = from_config_load_ip(service, sc);
    else if (type == "unix")
        transport = from_config_load_unix(service, sc);
    else
        throw transport_error(transport_error::not_supported);

    transport->set_password(password);

    return transport;
}

} // !irccd::transport_util
