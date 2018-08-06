/*
 * javascript_fixture.hpp -- test fixture helper for Javascript modules
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

#ifndef IRCCD_TEST_JAVASCRIPT_FIXTURE_HPP
#define IRCCD_TEST_JAVASCRIPT_FIXTURE_HPP

/**
 * \file javascript_fixture.hpp
 * \brief Test fixture helper for Javascript modules.
 */

#include "irccd_fixture.hpp"

#include <irccd/js/js_plugin.hpp>
#include <irccd/js/jsapi.hpp>

#include <irccd/js/directory_jsapi.hpp>
#include <irccd/js/elapsed_timer_jsapi.hpp>
#include <irccd/js/file_jsapi.hpp>
#include <irccd/js/irccd_jsapi.hpp>
#include <irccd/js/jsapi.hpp>
#include <irccd/js/logger_jsapi.hpp>
#include <irccd/js/plugin_jsapi.hpp>
#include <irccd/js/server_jsapi.hpp>
#include <irccd/js/system_jsapi.hpp>
#include <irccd/js/timer_jsapi.hpp>
#include <irccd/js/unicode_jsapi.hpp>
#include <irccd/js/util_jsapi.hpp>

namespace irccd::test {

/**
 * \brief Test fixture helper for Javascript modules.
 */
class javascript_fixture : public irccd_fixture {
protected:
    /**
     * \brief Javascript plugin.
     */
    std::shared_ptr<js_plugin> plugin_;

    /**
     * Constructor.
     *
     * Initialize a Javascript plugin with all Javascript API modules.
     *
     * \param path the path to a Javascript file if required
     */
    javascript_fixture(const std::string& path = "");
};

} // !irccd::test

#endif // !IRCCD_TEST_JAVASCRIPT_FIXTURE_HPP
