/*
 * mock.cpp -- keep track of function invocations
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

#include "mock.hpp"

namespace irccd::test {

void mock::push(std::string name, args args) const
{
	table_[name].push_back(std::move(args));
}

auto mock::find(const std::string& name) const -> std::vector<args>
{
	if (const auto it = table_.find(name); it != table_.end())
		return it->second;

	return {};
}

void mock::clear(const std::string& name) const noexcept
{
	table_.erase(name);
}

void mock::clear() const noexcept
{
	table_.clear();
}

auto mock::empty() const noexcept -> bool
{
	return table_.empty();
}

} // !irccd::test
