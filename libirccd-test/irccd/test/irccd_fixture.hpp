/*
 * irccd_fixture.hpp -- test fixture for irccd
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

#ifndef IRCCD_TEST_IRCCD_FIXTURE_HPP
#define IRCCD_TEST_IRCCD_FIXTURE_HPP

/**
 * \file irccd_fixture.hpp
 * \brief Test fixture for irccd.
 */

#include <irccd/daemon/irccd.hpp>

namespace irccd::test {

/**
 * \brief Test fixture for irccd.
 */
class irccd_fixture {
protected:
    /**
     * \brief Boost.Asio context.
     */
    boost::asio::io_context ctx_;

    /**
     * \brief Main irccd daemon.
     */
    irccd irccd_{ctx_};

    /**
     * Default constructor.
     *
     * Initialize the logger with a silent sink.
     */
    irccd_fixture();
};

} // !irccd::test

#endif // !IRCCD_TEST_IRCCD_FIXTURE_HPP
