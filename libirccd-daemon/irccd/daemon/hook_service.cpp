/*
 * hook_service.cpp -- irccd hook service
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

#include <algorithm>

#include "hook_service.hpp"

using std::find;
using std::move;

namespace irccd::daemon {

hook_service::hook_service(bot& bot) noexcept
	: bot_(bot)
{
}

auto hook_service::has(const hook& hook) const noexcept -> bool
{
	return find(hooks_.begin(), hooks_.end(), hook) != hooks_.end();
}

void hook_service::add(hook hook)
{
	if (has(hook))
		throw hook_error(hook_error::already_exists, hook.get_id(), "");

	hooks_.push_back(move(hook));
}

void hook_service::remove(const hook& hook) noexcept
{
	hooks_.erase(std::remove(hooks_.begin(), hooks_.end(), hook), hooks_.end());
}

auto hook_service::list() const noexcept -> const hooks&
{
	return hooks_;
}

auto hook_service::list() noexcept -> hooks&
{
	return hooks_;
}

void hook_service::clear() noexcept
{
	hooks_.clear();
}

} // !irccd::daemon
