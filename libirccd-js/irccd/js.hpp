/*
 * js.hpp -- libirccd-js convenience header
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

#ifndef IRCCD_JS_HPP
#define IRCCD_JS_HPP

/**
 * \file js.hpp
 * \brief libirccd-js convenience header.
 */

#include "sysconfig.hpp"

#include "js/api.hpp"
#include "js/chrono_api.hpp"
#include "js/directory_api.hpp"
#include "js/duk.hpp"
#include "js/file_api.hpp"
#include "js/irccd_api.hpp"
#include "js/logger_api.hpp"
#include "js/plugin.hpp"
#include "js/plugin_api.hpp"
#include "js/server_api.hpp"
#include "js/system_api.hpp"
#include "js/timer_api.hpp"
#include "js/unicode.hpp"
#include "js/unicode_api.hpp"
#include "js/util_api.hpp"

namespace irccd {

/**
 * \brief Javascript namespace.
 */
namespace js {

} // !js

} // !irccd

#endif // !IRCCD_JS_HPP
