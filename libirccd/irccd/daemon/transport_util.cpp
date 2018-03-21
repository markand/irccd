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

#include <irccd/ini.hpp>
#include <irccd/string_util.hpp>

#include <irccd/daemon/ip_transport_server.hpp>

#if !defined(IRCCD_SYSTEM_WINDOWS)
#   include <irccd/daemon/local_transport_server.hpp>
#endif

#if defined(HAVE_SSL)
#   include <irccd/daemon/tls_transport_server.hpp>
#endif

#include "transport_util.hpp"

namespace irccd {

namespace transport_util {

namespace {

std::unique_ptr<transport_server> load_transport_ip(boost::asio::io_service& service,
                                                    const ini::section& sc)
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

std::unique_ptr<transport_server> load_transport_unix(boost::asio::io_service& service,
                                                      const ini::section& sc)
{
    assert(sc.key() == "transport");

#if !defined(IRCCD_SYSTEM_WINDOWS)
    using boost::asio::local::stream_protocol;

    ini::section::const_iterator it = sc.find("path");

    if (it == sc.end())
        throw std::invalid_argument("missing 'path' parameter");

    // Remove the file first.
    std::remove(it->value().c_str());

    stream_protocol::endpoint endpoint(it->value());
    stream_protocol::acceptor acceptor(service, std::move(endpoint));

    return std::make_unique<local_transport_server>(std::move(acceptor));
#else
    (void)service;
    (void)sc;

    throw std::invalid_argument("unix transports not supported on on this platform");
#endif
}

} // !namespace

std::unique_ptr<transport_server> from_config(boost::asio::io_service& service, const ini::section& sc)
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

} // !transport_util

} // !irccd
