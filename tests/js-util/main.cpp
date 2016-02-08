/*
 * main.cpp -- test Irccd.Util API
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

#include <js-irccd.h>
#include <js-util.h>

using namespace irccd;

TEST(TestJsUtil, formatSimple)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsUtil(ctx);

	try {
		ctx.peval(js::Script{
			"result = Irccd.Util.format(\"#{target}\", { target: \"markand\" })"
		});

	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsUtil, splituser)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsUtil(ctx);

	try {
		ctx.peval(js::Script{"result = Irccd.Util.splituser(\"user!~user@hyper/super/host\");"});

		ASSERT_EQ("user", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsUtil, splithost)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsUtil(ctx);

	try {
		ctx.peval(js::Script{"result = Irccd.Util.splithost(\"user!~user@hyper/super/host\");"});

		ASSERT_EQ("!~user@hyper/super/host", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
