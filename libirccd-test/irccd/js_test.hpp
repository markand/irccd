/*
 * js_test.hpp -- test fixture helper for Javascript modules
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_JS_TEST_HPP
#define IRCCD_JS_TEST_HPP

/**
 * \file js_test.hpp
 * \brief Test fixture helper for Javascript modules.
 */

#include <irccd/irccd.hpp>
#include <irccd/js_plugin.hpp>
#include <irccd/js_irccd_module.hpp>

#include "journal_server.hpp"

namespace irccd {

/**
 * \brief Test fixture helper for Javascript modules.
 */
template <typename... Modules>
class js_test {
private:
    template <typename Module>
    inline void add()
    {
        Module().load(irccd_, plugin_);
    }

    template <typename M1, typename M2, typename... Tail>
    inline void add()
    {
        add<M1>();
        add<M2, Tail...>();
    }

public:
    irccd irccd_;                               //!< Irccd instance.
    std::shared_ptr<js_plugin> plugin_;         //!< Javascript plugin.
    std::shared_ptr<journal_server> server_;    //!< A journal server.

    /**
     * Constructor.
     *
     * Create a server and a test plugin.
     */
    js_test(const std::string& plugin_path = IRCCD_PLUGIN_TEST)
        : plugin_(new js_plugin("test", plugin_path))
        , server_(new journal_server("test"))
    {
        // Irccd is mandatory at the moment.
        add<js_irccd_module>();
        add<Modules...>();

        plugin_->on_load(irccd_);

        // Add some CMake variables.
        duk_push_string(plugin_->context(), TESTS_BINARY_DIR);
        duk_put_global_string(plugin_->context(), "TESTS_BINARY_DIR");
    }
};

} // !irccd

#endif // !IRCCD_JS_TEST_HPP