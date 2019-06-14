/*
 * irccd_fixture.cpp -- test fixture for irccd
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

#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/plugin_service.hpp>

#include "irccd_fixture.hpp"
#include "test_plugin_loader.hpp"

using irccd::daemon::logger::silent_sink;

using std::make_unique;

namespace irccd::test {

irccd_fixture::irccd_fixture()
{
	bot_.set_log(make_unique<silent_sink>());
	bot_.get_plugins().add_loader(make_unique<test_plugin_loader>());
}

} // !irccd
