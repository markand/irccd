/*
 * main.cpp -- test Irccd API
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <gtest/gtest.h>

#include <irccd-config.h>

#include <irccd/js-irccd.h>
#include <irccd/logger.h>

using namespace irccd;

TEST(TestJsIrccd, version)
{
	duk::Context ctx;

	loadJsIrccd(ctx);

	try {
		auto ret = duk::pevalString(ctx,
			"major = Irccd.version.major;"
			"minor = Irccd.version.minor;"
			"patch = Irccd.version.patch;"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(IRCCD_VERSION_MAJOR, duk::getGlobal<int>(ctx, "major"));
		ASSERT_EQ(IRCCD_VERSION_MINOR, duk::getGlobal<int>(ctx, "minor"));
		ASSERT_EQ(IRCCD_VERSION_PATCH, duk::getGlobal<int>(ctx, "patch"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
