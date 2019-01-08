/*
 * transport_client.cpp -- server side transport clients
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

#include "transport_client.hpp"
#include "transport_server.hpp"

namespace irccd::daemon {

void transport_client::flush()
{
	if (queue_.empty())
		return;

	const auto self = shared_from_this();

	stream_->send(queue_.front().first, [this, self] (auto code) {
		if (queue_.front().second)
			queue_.front().second(code);

		queue_.pop_front();

		if (code)
			erase();
		else
			flush();
	});
}

void transport_client::erase()
{
	state_ = state::closing;

	if (auto parent = parent_.lock())
		parent->get_clients().erase(shared_from_this());
}

transport_client::transport_client(std::weak_ptr<transport_server> server,
                                   std::shared_ptr<stream> stream) noexcept
	: parent_(server)
	, stream_(std::move(stream))
{
	assert(stream_);
}

auto transport_client::get_state() const noexcept -> state
{
	return state_;
}

void transport_client::set_state(state state) noexcept
{
	state_ = state;
}

void transport_client::read(stream::recv_handler handler)
{
	assert(handler);

	if (state_ != state::closing) {
		const auto self = shared_from_this();

		stream_->recv([this, self, handler] (auto code, auto msg) {
			handler(code, msg);

			if (code)
				erase();
		});
	}
}

void transport_client::write(nlohmann::json json, stream::send_handler handler)
{
	const auto in_progress = queue_.size() > 0;

	queue_.emplace_back(std::move(json), std::move(handler));

	if (!in_progress)
		flush();
}

void transport_client::success(const std::string& cname, stream::send_handler handler)
{
	assert(!cname.empty());

	write({{ "command", cname }}, std::move(handler));
}

void transport_client::error(std::error_code code, stream::send_handler handler)
{
	error(std::move(code), "", std::move(handler));
}

void transport_client::error(std::error_code code, std::string_view cname, stream::send_handler handler)
{
	assert(code);

	auto json = nlohmann::json::object({
		{ "error",              code.value()            },
		{ "errorCategory",      code.category().name()  },
		{ "errorMessage",       code.message()          }
	});

	// TODO: check newer version of JSON for string_view support.
	if (!cname.empty())
		json["command"] = std::string(cname);

	const auto self = shared_from_this();

	write(std::move(json), [this, handler, self] (auto code) {
		erase();

		if (handler)
			handler(code);
	});

	state_ = state::closing;
}

} // !irccd::daemon
