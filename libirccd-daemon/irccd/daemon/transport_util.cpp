/*
 * transport_util.cpp -- transport utilities
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#include <irccd/acceptor.hpp>
#include <irccd/ini_util.hpp>
#include <irccd/string_util.hpp>

#include "transport_util.hpp"
#include "transport_server.hpp"

namespace asio = boost::asio;

namespace irccd::daemon::transport_util {

namespace {

auto from_config_load_ip_protocols(const ini::section& sc) -> std::pair<bool, bool>
{
	bool ipv4 = true, ipv6 = true;

	if (const auto it = sc.find("ipv4"); it != sc.end())
		ipv4 = string_util::is_boolean(it->get_value());
	if (const auto it = sc.find("ipv6"); it != sc.end())
		ipv6 = string_util::is_boolean(it->get_value());

	if (!ipv4 && !ipv6)
		throw transport_error(transport_error::invalid_family);

	return { ipv4, ipv6 };
}

#if defined(IRCCD_HAVE_SSL)

auto from_config_load_ssl(const ini::section& sc) -> boost::asio::ssl::context
{
	const auto key = sc.get("key").get_value();
	const auto cert = sc.get("certificate").get_value();

	if (key.empty())
		throw transport_error(transport_error::invalid_private_key);
	if (cert.empty())
		throw transport_error(transport_error::invalid_certificate);

	boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);

	ctx.use_private_key_file(key, boost::asio::ssl::context::pem);
	ctx.use_certificate_file(cert, boost::asio::ssl::context::pem);

	return ctx;
}

#endif // !IRCCD_HAVE_SSL

auto from_config_load_ip(asio::io_context& service, const ini::section& sc) -> std::unique_ptr<acceptor>
{
	assert(sc.get_key() == "transport");

	const auto port = ini_util::get_uint<std::uint16_t>(sc, "port");
	const auto address = ini_util::optional_string(sc, "address", "*");
	const auto [ ipv4, ipv6 ] = from_config_load_ip_protocols(sc);

	if (!port)
		throw transport_error(transport_error::invalid_port);
	if (address.empty())
		throw transport_error(transport_error::invalid_address);

	if (string_util::is_boolean(sc.get("ssl").get_value()))
#if !defined(IRCCD_HAVE_SSL)
	throw transport_error(transport_error::ssl_disabled);
#else
		return std::make_unique<tls_ip_acceptor>(from_config_load_ssl(sc),
			service, address, *port, ipv4, ipv6);
#endif // !IRCCD_HAVE_SSL

	return std::make_unique<ip_acceptor>(service, address, *port, ipv4, ipv6);
}

auto from_config_load_local(asio::io_context& service, const ini::section& sc) -> std::unique_ptr<acceptor>
{
	assert(sc.get_key() == "transport");

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
	const auto path = sc.get("path").get_value();

	if (path.empty())
		throw transport_error(transport_error::invalid_path);

	if (string_util::is_boolean(sc.get("ssl").get_value()))
#if !defined(IRCCD_HAVE_SSL)
	throw transport_error(transport_error::ssl_disabled);
#else
		return std::make_unique<tls_local_acceptor>(from_config_load_ssl(sc), service, path);
#endif // !IRCCD_HAVE_SSL

	return std::make_unique<local_acceptor>(service, path);
#else
	(void)service;
	(void)sc;

	throw transport_error(transport_error::not_supported);
#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS
}

} // !namespace

auto from_config(asio::io_context& service, const ini::section& sc) -> std::unique_ptr<transport_server>
{
	assert(sc.get_key() == "transport");

	const auto type = sc.get("type").get_value();
	const auto password = sc.get("password").get_value();

	if (type.empty())
		throw transport_error(transport_error::not_supported);

	std::unique_ptr<acceptor> acceptor;

	if (type == "ip")
		acceptor = from_config_load_ip(service, sc);
	else if (type == "unix")
		acceptor = from_config_load_local(service, sc);
	else
		throw transport_error(transport_error::not_supported);

	auto transport = std::make_unique<transport_server>(std::move(acceptor));

	transport->set_password(password);

	return transport;
}

} // !irccd::daemon::transport_util
