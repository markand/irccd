/*
 * plugin_fixture.cpp -- test fixture helper for Javascript plugins
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

#include <cassert>

#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/plugin_service.hpp>
#include <irccd/daemon/server_service.hpp>

#include <irccd/js/directory_api.hpp>
#include <irccd/js/elapsed_timer_api.hpp>
#include <irccd/js/file_api.hpp>
#include <irccd/js/irccd_api.hpp>
#include <irccd/js/plugin.hpp>
#include <irccd/js/logger_api.hpp>
#include <irccd/js/plugin_api.hpp>
#include <irccd/js/server_api.hpp>
#include <irccd/js/system_api.hpp>
#include <irccd/js/timer_api.hpp>
#include <irccd/js/unicode_api.hpp>
#include <irccd/js/util_api.hpp>

#include "js_plugin_fixture.hpp"

using irccd::js::plugin;

using irccd::daemon::logger::silent_sink;

namespace irccd::test {

js_plugin_fixture::js_plugin_fixture(std::string path)
	: server_(std::make_shared<mock_server>(service_, "test", "local"))
{
	plugin_ = std::make_unique<plugin>("test", std::move(path));

	bot_.set_log(std::make_unique<silent_sink>());
	bot_.get_log().set_verbose(false);
	bot_.plugins().add(plugin_);
	bot_.servers().add(server_);

	server_->disconnect();
	server_->set_nickname("irccd");
	server_->clear();

	for (const auto& f : js::api::registry())
		f()->load(bot_, *plugin_);

	plugin_->open();
}

} // !irccd::test
