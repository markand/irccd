/*
 * plugin_test.hpp -- test fixture helper for Javascript plugins
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

#ifndef IRCCD_TEST_PLUGIN_TEST_HPP
#define IRCCD_TEST_PLUGIN_TEST_HPP

/**
 * \file plugin_test.hpp
 * \brief test fixture helper for Javascript plugins.
 */

#include <boost/asio/io_service.hpp>

#include <irccd/daemon/irccd.hpp>

#include <irccd/js/js_plugin.hpp>

#include "journal_server.hpp"

namespace irccd {

/**
 * \brief test fixture helper for Javascript plugins.
 *
 * Holds a plugin that is opened (but not loaded).
 */
class plugin_test {
protected:
    boost::asio::io_service service_;
    irccd irccd_{service_};
    std::shared_ptr<js_plugin> plugin_;
    std::shared_ptr<journal_server> server_;

public:
    /**
     * Construct the fixture test.
     *
     * \param path the full plugin path (e.g. /usr/lib64/irccd/ask.js)
     */
    plugin_test(std::string path);
};

} // !irccd

#endif // !IRCCD_TEST_PLUGIN_TESTER_HPP
