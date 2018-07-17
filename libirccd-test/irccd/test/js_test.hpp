/*
 * js_test.hpp -- test fixture helper for Javascript modules
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

#ifndef IRCCD_TEST_JS_TEST_HPP
#define IRCCD_TEST_JS_TEST_HPP

/**
 * \file js_test.hpp
 * \brief Test fixture helper for Javascript modules.
 */

#include <boost/asio.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>

#include <irccd/js/js_plugin.hpp>
#include <irccd/js/irccd_jsapi.hpp>

#include "journal_server.hpp"

namespace irccd {

/**
 * \brief Test fixture helper for Javascript modules.
 */
template <typename... Modules>
class js_test {
private:
    template <typename Module>
    void add();

    template <typename M1, typename M2, typename... Tail>
    void add();

public:
    boost::asio::io_service service_;
    irccd irccd_{service_};                     //!< Irccd instance.
    std::shared_ptr<js_plugin> plugin_;         //!< Javascript plugin.
    std::shared_ptr<journal_server> server_;    //!< A journal server.

    /**
     * Constructor.
     *
     * Create a server and a test plugin.
     */
    js_test(const std::string& plugin_path = "");
};

template <typename... Modules>
template <typename Module>
void js_test<Modules...>::add()
{
    Module().load(irccd_, plugin_);
}

template <typename... Modules>
template <typename M1, typename M2, typename... Tail>
void js_test<Modules...>::add()
{
    add<M1>();
    add<M2, Tail...>();
}

template <typename... Modules>
js_test<Modules...>::js_test(const std::string& plugin_path)
    : plugin_(new js_plugin(plugin_path))
    , server_(new journal_server(service_, "test"))
{
    irccd_.set_log(std::make_unique<logger::silent_sink>());

    // Irccd is mandatory at the moment.
    add<irccd_jsapi>();
    add<Modules...>();

    // Add some CMake variables.
    duk_push_string(plugin_->get_context(), CMAKE_BINARY_DIR);
    duk_put_global_string(plugin_->get_context(), "CMAKE_BINARY_DIR");
    duk_push_string(plugin_->get_context(), CMAKE_SOURCE_DIR);
    duk_put_global_string(plugin_->get_context(), "CMAKE_SOURCE_DIR");
    duk_push_string(plugin_->get_context(), CMAKE_CURRENT_BINARY_DIR);
    duk_put_global_string(plugin_->get_context(), "CMAKE_CURRENT_BINARY_DIR");
    duk_push_string(plugin_->get_context(), CMAKE_CURRENT_SOURCE_DIR);
    duk_put_global_string(plugin_->get_context(), "CMAKE_CURRENT_SOURCE_DIR");

    if (!plugin_path.empty())
        plugin_->open();
}

} // !irccd

#endif // !IRCCD_TEST_JS_TEST_HPP
