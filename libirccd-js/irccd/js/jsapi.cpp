/*
 * jsapi.cpp -- Javascript API module
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

#include "directory_jsapi.hpp"
#include "elapsed_timer_jsapi.hpp"
#include "file_jsapi.hpp"
#include "irccd_jsapi.hpp"
#include "logger_jsapi.hpp"
#include "plugin_jsapi.hpp"
#include "server_jsapi.hpp"
#include "system_jsapi.hpp"
#include "timer_jsapi.hpp"
#include "unicode_jsapi.hpp"
#include "util_jsapi.hpp"

namespace irccd {

namespace {

template <typename T>
auto bind() noexcept -> jsapi::factory
{
    return [] () noexcept {
        return std::make_unique<T>();
    };
}

} // !namespace

const std::vector<jsapi::factory> jsapi::registry{
    // Irccd API must be loaded first.
    bind<irccd_jsapi>(),
    bind<directory_jsapi>(),
    bind<elapsed_timer_jsapi>(),
    bind<file_jsapi>(),
    bind<logger_jsapi>(),
    bind<plugin_jsapi>(),
    bind<server_jsapi>(),
    bind<system_jsapi>(),
    bind<timer_jsapi>(),
    bind<unicode_jsapi>(),
    bind<util_jsapi>()
};

} // !irccd
