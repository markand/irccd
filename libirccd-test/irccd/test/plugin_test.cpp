/*
 * plugin_test.cpp -- test fixture helper for Javascript plugins
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

#include <cassert>

#include <irccd/daemon/logger.hpp>

#include <irccd/daemon/service/plugin_service.hpp>
#include <irccd/daemon/service/server_service.hpp>

#include <irccd/js/directory_jsapi.hpp>
#include <irccd/js/elapsed_timer_jsapi.hpp>
#include <irccd/js/file_jsapi.hpp>
#include <irccd/js/irccd_jsapi.hpp>
#include <irccd/js/js_plugin.hpp>
#include <irccd/js/logger_jsapi.hpp>
#include <irccd/js/plugin_jsapi.hpp>
#include <irccd/js/server_jsapi.hpp>
#include <irccd/js/system_jsapi.hpp>
#include <irccd/js/timer_jsapi.hpp>
#include <irccd/js/unicode_jsapi.hpp>
#include <irccd/js/util_jsapi.hpp>

#include "plugin_test.hpp"

namespace irccd {

plugin_test::plugin_test(std::string path)
    : server_(std::make_shared<journal_server>(service_, "test", "local"))
{
    server_->set_nickname("irccd");
    plugin_ = std::make_unique<js_plugin>("test", std::move(path));

    irccd_.set_log(std::make_unique<logger::silent_sink>());
    irccd_.get_log().set_verbose(false);
    irccd_.plugins().add(plugin_);
    irccd_.servers().add(server_);

    irccd_jsapi().load(irccd_, plugin_);
    directory_jsapi().load(irccd_, plugin_);
    elapsed_timer_jsapi().load(irccd_, plugin_);
    file_jsapi().load(irccd_, plugin_);
    logger_jsapi().load(irccd_, plugin_);
    plugin_jsapi().load(irccd_, plugin_);
    server_jsapi().load(irccd_, plugin_);
    system_jsapi().load(irccd_, plugin_);
    timer_jsapi().load(irccd_, plugin_);
    unicode_jsapi().load(irccd_, plugin_);
    util_jsapi().load(irccd_, plugin_);

    plugin_->open();
}

} // !irccd
