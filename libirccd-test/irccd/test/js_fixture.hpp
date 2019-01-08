/*
 * js_fixture.hpp -- test fixture helper for Javascript modules
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

#ifndef IRCCD_TEST_JS_FIXTURE_HPP
#define IRCCD_TEST_JS_FIXTURE_HPP

/**
 * \file js_fixture.hpp
 * \brief Test fixture helper for Javascript modules.
 */

#include "irccd_fixture.hpp"

#include <irccd/js/plugin.hpp>
#include <irccd/js/api.hpp>

#include <irccd/js/irccd_api.hpp>

namespace irccd::test {

/**
 * \brief Test fixture helper for Javascript modules.
 */
class js_fixture : public irccd_fixture {
protected:
	/**
	 * \brief Javascript plugin.
	 */
	std::shared_ptr<js::plugin> plugin_;

	/**
	 * Constructor.
	 *
	 * Initialize a Javascript plugin with all Javascript API modules.
	 *
	 * \param path the path to a Javascript file if required
	 */
	js_fixture(const std::string& path = "");
};

} // !irccd::test

#endif // !IRCCD_TEST_JS_FIXTURE_HPP
