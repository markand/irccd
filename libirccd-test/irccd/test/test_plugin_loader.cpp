/*
 * test_plugin_loader.cpp -- special plugin loader for unit tests
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

#include "broken_plugin.hpp"
#include "mock_plugin.hpp"
#include "test_plugin_loader.hpp"

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::string_view;

using irccd::daemon::plugin;

namespace irccd::test {

namespace {

auto create(string_view id) -> shared_ptr<plugin>
{
	string strid(id);

	if (id == "broken")
		return make_shared<broken_plugin>(strid);
	if (id == "mock")
		return make_shared<mock_plugin>(strid);

	return nullptr;
}

} // !namespace

auto test_plugin_loader::open(string_view id, string_view) -> shared_ptr<plugin>
{
	return create(id);
}

auto test_plugin_loader::find(string_view id) -> shared_ptr<plugin>
{
	return create(id);
}

} // !irccd::test
