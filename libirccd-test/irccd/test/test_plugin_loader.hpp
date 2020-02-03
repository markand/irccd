/*
 * test_plugin_loader.hpp -- special plugin loader for unit tests
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

#ifndef IRCCD_TEST_PLUGIN_LOADER_HPP
#define IRCCD_TEST_PLUGIN_LOADER_HPP

/**
 * \file test_plugin_loader.hpp
 * \brief Special plugin loader for unit tests.
 */

#include <irccd/daemon/plugin.hpp>

namespace irccd::test {

/**
 * \brief Special plugin loader for unit tests.
 *
 * This class reimplements the functions find and open to return special plugin
 * objects.
 *
 * The following names are supported:
 *
 * - broken: will instanciate a broken_plugin object
 * - mock: will instanciate a mock_plugin object
 */
class test_plugin_loader : public daemon::plugin_loader {
public:
	/**
	 * \copydoc daemon::plugin_loader::open
	 */
	auto open(std::string_view id, std::string_view file) -> std::shared_ptr<daemon::plugin> override;

	/**
	 * \copydoc daemon::plugin_loader::find
	 */
	auto find(std::string_view id) -> std::shared_ptr<daemon::plugin> override;
};

} // !irccd::test

#endif // !IRCCD_TEST_PLUGIN_LOADER_HPP
