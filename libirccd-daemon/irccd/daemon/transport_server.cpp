/*
 * transport_server.cpp -- server side transports
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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
#include <system_error>

#include "transport_client.hpp"
#include "transport_server.hpp"

namespace irccd::daemon {

transport_server::transport_server(std::unique_ptr<acceptor> acceptor) noexcept
	: acceptor_(std::move(acceptor))
{
	assert(acceptor_);
}

auto transport_server::get_clients() const noexcept -> const client_set&
{
	return clients_;
}

auto transport_server::get_clients() noexcept -> client_set&
{
	return clients_;
}

auto transport_server::get_password() const noexcept -> const std::string&
{
	return password_;
}

void transport_server::set_password(std::string password) noexcept
{
	password_ = std::move(password);
}

void transport_server::accept(accept_handler handler)
{
	acceptor_->accept([this, handler] (auto code, auto stream) {
		if (code)
			handler(code, nullptr);
		else
			handler(code, std::make_shared<transport_client>(shared_from_this(), std::move(stream)));
	});
}

transport_error::transport_error(error code) noexcept
	: system_error(make_error_code(code))
{
}

auto transport_category() noexcept -> const std::error_category&
{
	static const class category : public std::error_category {
	public:
		auto name() const noexcept -> const char* override
		{
			return "transport";
		}

		auto message(int e) const -> std::string override
		{
			switch (static_cast<transport_error::error>(e)) {
			case transport_error::auth_required:
				return "authentication required";
			case transport_error::invalid_auth:
				return "invalid authentication";
			case transport_error::invalid_port:
				return "invalid port";
			case transport_error::invalid_address:
				return "invalid address";
			case transport_error::invalid_hostname:
				return "invalid hostname";
			case transport_error::invalid_path:
				return "invalid socket path";
			case transport_error::invalid_family:
				return "invalid family";
			case transport_error::invalid_certificate:
				return "invalid certificate";
			case transport_error::invalid_private_key:
				return "invalid private key";
			case transport_error::ssl_disabled:
				return "ssl is not enabled";
			case transport_error::not_supported:
				return "transport not supported";
			default:
				return "no error";
			}
		}
	} category;

	return category;
};

auto make_error_code(transport_error::error e) noexcept -> std::error_code
{
	return { static_cast<int>(e), transport_category() };
}

} // !irccd::daemon
