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

#include <irccd/ini_util.hpp>
#include <irccd/string_util.hpp>

#include <irccd/daemon/ip_transport_server.hpp>

#if !defined(IRCCD_SYSTEM_WINDOWS)
#   include <irccd/daemon/local_transport_server.hpp>
#endif

#if defined(HAVE_SSL)
#   include <irccd/daemon/tls_transport_server.hpp>
#endif

#include "transport_util.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

namespace irccd {

namespace transport_util {

namespace {

tcp from_config_load_ip_protocol(const ini::section& sc)
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

    return ipv4 ? tcp::v4() : tcp::v6();
}

tcp::endpoint from_config_load_ip_endpoint(const ini::section& sc)
{
    const auto port = ini_util::get_uint<std::uint16_t>(sc, "port");
    const auto address = ini_util::optional_string(sc, "address", "*");

    if (!port)
        throw transport_error(transport_error::invalid_port);
    if (address.empty())
        throw transport_error(transport_error::invalid_address);

    const auto protocol = from_config_load_ip_protocol(sc);

    return address == "*"
        ? tcp::endpoint(protocol, *port)
        : tcp::endpoint(address::from_string(address), *port);
}

tcp::acceptor from_config_load_ip_acceptor(io_service& service, const ini::section& sc)
{
    return tcp::acceptor(service, from_config_load_ip_endpoint(sc), true);
}

std::unique_ptr<transport_server> from_config_load_ip(io_service& service, const ini::section& sc)
{
    assert(sc.key() == "transport");

    auto acceptor = from_config_load_ip_acceptor(service, sc);

    if (string_util::is_boolean(sc.get("ssl").value())) {
#if !defined(HAVE_SSL)
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

        return std::make_unique<tls_transport_server>(service, std::move(acceptor), std::move(ctx));
#endif
    }

    return std::make_unique<ip_transport_server>(service, std::move(acceptor));
}

std::unique_ptr<transport_server> from_config_load_unix(io_service& service, const ini::section& sc)
{
    assert(sc.key() == "transport");

#if !defined(IRCCD_SYSTEM_WINDOWS)
    using boost::asio::local::stream_protocol;

    const auto path = sc.get("path").value();

    if (path.empty())
        throw transport_error(transport_error::invalid_path);

    // Remove the file first.
    std::remove(path.c_str());

    stream_protocol::endpoint endpoint(path);
    stream_protocol::acceptor acceptor(service, std::move(endpoint));

    return std::make_unique<local_transport_server>(service, std::move(acceptor));
#else
    (void)service;
    (void)sc;

    throw transport_error(transport_error::not_supported);
#endif
}

} // !namespace

std::unique_ptr<transport_server> from_config(io_service& service, const ini::section& sc)
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

} // !transport_util

} // !irccd
