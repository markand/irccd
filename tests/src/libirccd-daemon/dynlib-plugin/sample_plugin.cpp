/*
 * sample_plugin.cpp -- basic exported plugin test
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

#include <irccd/daemon/dynlib_plugin.hpp>

#include <irccd/test/mock_plugin.hpp>

using irccd::test::mock_plugin;

namespace irccd {

class sample_plugin : public mock_plugin {
public:
	sample_plugin()
		: mock_plugin("test")
	{
	}

	static auto abi() -> version
	{
		return version();
	}

	static auto init(std::string) -> std::unique_ptr<plugin>
	{
		return std::make_unique<sample_plugin>();
	}
};

BOOST_DLL_ALIAS(sample_plugin::abi, irccd_abi_sample_plugin)
BOOST_DLL_ALIAS(sample_plugin::init, irccd_init_sample_plugin)

} // !irccd
