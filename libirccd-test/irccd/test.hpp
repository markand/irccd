/*
 * test.hpp -- libirccd-test convenience header
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

#ifndef IRCCD_TEST_HPP
#define IRCCD_TEST_HPP

/**
 * \file test.hpp
 * \brief libirccd-test convenience header.
 */

#include "sysconfig.hpp"

#include "test/broken_plugin.hpp"
#include "test/cli_fixture.hpp"
#include "test/command_fixture.hpp"
#include "test/debug_server.hpp"
#include "test/irccd_fixture.hpp"
#include "test/js_fixture.hpp"
#include "test/js_plugin_fixture.hpp"
#include "test/mock.hpp"
#include "test/mock_plugin.hpp"
#include "test/mock_server.hpp"
#include "test/mock_stream.hpp"
#include "test/test_plugin_loader.hpp"

namespace irccd {

/**
 * \brief Namespace for unit tests.
 */
namespace test {

} // !test

} // !irccd

#endif // !IRCCD_TEST_HPP
