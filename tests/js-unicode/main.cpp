/*
 * main.cpp -- test Irccd.Unicode API
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

/*
 * /!\ Be sure that this file is kept saved in UTF-8 /!\
 */

#include <gtest/gtest.h>

#include <irccd/js-irccd.hpp>
#include <irccd/js-unicode.hpp>

using namespace irccd;

TEST(TestJsUnicode, isLetter)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsUnicode(ctx);

	try {
		duk::pevalString(ctx, "result = Irccd.Unicode.isLetter(String('é').charCodeAt(0));");
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "result"));

		duk::pevalString(ctx, "result = Irccd.Unicode.isLetter(String('€').charCodeAt(0));");
		ASSERT_FALSE(duk::getGlobal<bool>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsUnicode, isLower)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsUnicode(ctx);

	try {
		duk::pevalString(ctx, "result = Irccd.Unicode.isLower(String('é').charCodeAt(0));");
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "result"));

		duk::pevalString(ctx, "result = Irccd.Unicode.isLower(String('É').charCodeAt(0));");
		ASSERT_FALSE(duk::getGlobal<bool>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsUnicode, isUpper)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsUnicode(ctx);

	try {
		duk::pevalString(ctx, "result = Irccd.Unicode.isUpper(String('É').charCodeAt(0));");
		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "result"));

		duk::pevalString(ctx, "result = Irccd.Unicode.isUpper(String('é').charCodeAt(0));");
		ASSERT_FALSE(duk::getGlobal<bool>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
