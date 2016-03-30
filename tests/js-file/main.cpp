/*
 * main.cpp -- test Irccd.File API
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

#include <fstream>

#include <gtest/gtest.h>

#include <irccd/js-file.h>
#include <irccd/js-irccd.h>

using namespace irccd;

TEST(TestJsFile, functionBasename)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		if (duk::pevalString(ctx, "result = Irccd.File.basename('/usr/local/etc/irccd.conf');") != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("irccd.conf", duk::getGlobal<std::string>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionDirname)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::pevalString(ctx, "result = Irccd.File.dirname('/usr/local/etc/irccd.conf');");

		ASSERT_EQ("/usr/local/etc", duk::getGlobal<std::string>(ctx,"result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionExists)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);
		duk::pevalString(ctx, "result = Irccd.File.exists(directory + '/file.txt')");

		ASSERT_TRUE(duk::getGlobal<bool>(ctx,"result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionExists2)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::pevalString(ctx, "result = Irccd.File.exists('file_which_does_not_exist.txt')");

		ASSERT_FALSE(duk::getGlobal<bool>(ctx,"result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionRemove)
{
	// First create a dummy file
	{
		std::ofstream out("test-js-fs.remove");
	}

	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		if (duk::pevalString(ctx, "Irccd.File.remove('test-js-fs.remove');") != 0)
			throw duk::error(ctx, -1);
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}

	std::ifstream in("test-js-fs.remove");

	ASSERT_FALSE(in.is_open());
}

TEST(TestJsFile, methodBasename)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx,"directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"result = f.basename();"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("file-1.txt", duk::getGlobal<std::string>(ctx,"result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodBasenameClosed)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx,"directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"f.close();"
			"result = f.basename();"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("file-1.txt", duk::getGlobal<std::string>(ctx,"result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodDirname)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx,"directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"result = f.dirname();"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(std::string{IRCCD_TESTS_DIRECTORY "/level-1"}, duk::getGlobal<std::string>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodDirnameClosed)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"f.close();"
			"result = f.dirname();"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(std::string{IRCCD_TESTS_DIRECTORY "/level-1"}, duk::getGlobal<std::string>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek1)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.seek(Irccd.File.SeekSet, 4);"
			"result = f.read(1);"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(".", duk::getGlobal<std::string>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}


TEST(TestJsFile, methodSeek1Closed)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.close();"
			"f.seek(Irccd.File.SeekSet, 4);"
			"result = f.read(1);"
			"result = typeof (result) === \"undefined\";"
		);


		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek2)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.seek(Irccd.File.SeekSet, 2);"
			"f.seek(Irccd.File.SeekCur, 2);"
			"result = f.read(1);"
		);


		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ(".", duk::getGlobal<std::string>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek2Closed)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.close();"
			"f.seek(Irccd.File.SeekSet, 2);"
			"f.seek(Irccd.File.SeekCur, 2);"
			"result = f.read(1);"
			"result = typeof (result) === \"undefined\";"
		);


		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek3)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.seek(Irccd.File.SeekEnd, -2);"
			"result = f.read(1);"
		);


		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_EQ("x", duk::getGlobal<std::string>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek3Closed)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.close();"
			"f.seek(Irccd.File.SeekEnd, -2);"
			"result = f.read(1);"
			"result = typeof (result) === \"undefined\";"
		);


		if (ret != 0)
			throw duk::error(ctx, -1);

		ASSERT_TRUE(duk::getGlobal<bool>(ctx, "result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodReadline)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"lines = [];"
			"f = new Irccd.File(directory + '/lines.txt', 'r');"
			"for (var s; s = f.readline(); ) {"
			"  lines.push(s);"
			"}"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		std::vector<std::string> expected{"a", "b", "c"};

		ASSERT_EQ(expected, duk::getGlobal<std::vector<std::string>>(ctx, "lines"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodReadlineClosed)
{
	duk::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		duk::putGlobal(ctx, "directory", IRCCD_TESTS_DIRECTORY);

		auto ret = duk::pevalString(ctx,
			"lines = [];"
			"f = new Irccd.File(directory + '/lines.txt', 'r');"
			"f.close();"
			"for (var s; s = f.readline(); ) {"
			"  lines.push(s);"
			"}"
		);

		if (ret != 0)
			throw duk::error(ctx, -1);

		std::vector<std::string> expected;

		ASSERT_EQ(expected, duk::getGlobal<std::vector<std::string>>(ctx, "lines"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
