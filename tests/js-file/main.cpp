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

#include <js-file.h>
#include <js-irccd.h>

using namespace irccd;

TEST(TestJsFile, functionBasename)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.peval(js::Script{"result = Irccd.File.basename('/usr/local/etc/irccd.conf');"});

		ASSERT_EQ("irccd.conf", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionDirname)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.peval(js::Script{"result = Irccd.File.dirname('/usr/local/etc/irccd.conf');"});

		ASSERT_EQ("/usr/local/etc", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionExists)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{"result = Irccd.File.exists(directory + '/file.txt')"});

		ASSERT_TRUE(ctx.getGlobal<bool>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, functionExists2)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.peval(js::Script{"result = Irccd.File.exists('file_which_does_not_exist.txt')"});

		ASSERT_FALSE(ctx.getGlobal<bool>("result"));
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

	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.peval(js::Script{"Irccd.File.remove('test-js-fs.remove');"});
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}

	std::ifstream in("test-js-fs.remove");

	ASSERT_FALSE(in.is_open());
}

TEST(TestJsFile, methodBasename)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"result = f.basename();"
		});

		ASSERT_EQ("file-1.txt", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodBasenameClosed)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"f.close();"
			"result = f.basename();"
		});

		ASSERT_EQ("file-1.txt", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodDirname)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"result = f.dirname();"
		});

		ASSERT_EQ(std::string{IRCCD_TESTS_DIRECTORY "/level-1"}, ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodDirnameClosed)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/level-1/file-1.txt', 'r');"
			"f.close();"
			"result = f.dirname();"
		});

		ASSERT_EQ(std::string{IRCCD_TESTS_DIRECTORY "/level-1"}, ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek1)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.seek(Irccd.File.SeekSet, 4);"
			"result = f.read(1);"
		});

		ASSERT_EQ(".", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}


TEST(TestJsFile, methodSeek1Closed)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.close();"
			"f.seek(Irccd.File.SeekSet, 4);"
			"result = f.read(1);"
			"result = typeof (result) === \"undefined\";"
		});

		ASSERT_TRUE(ctx.getGlobal<bool>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek2)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.seek(Irccd.File.SeekSet, 2);"
			"f.seek(Irccd.File.SeekCur, 2);"
			"result = f.read(1);"
		});

		ASSERT_EQ(".", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek2Closed)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.close();"
			"f.seek(Irccd.File.SeekSet, 2);"
			"f.seek(Irccd.File.SeekCur, 2);"
			"result = f.read(1);"
			"result = typeof (result) === \"undefined\";"
		});

		ASSERT_TRUE(ctx.getGlobal<bool>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek3)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.seek(Irccd.File.SeekEnd, -2);"
			"result = f.read(1);"
		});

		ASSERT_EQ("x", ctx.getGlobal<std::string>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodSeek3Closed)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"f = new Irccd.File(directory + '/file.txt', 'r');"
			"f.close();"
			"f.seek(Irccd.File.SeekEnd, -2);"
			"result = f.read(1);"
			"result = typeof (result) === \"undefined\";"
		});

		ASSERT_TRUE(ctx.getGlobal<bool>("result"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodReadline)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"lines = [];"
			"f = new Irccd.File(directory + '/lines.txt', 'r');"
			"for (var s; s = f.readline(); ) {"
			"  lines.push(s);"
			"}"
		});

		std::vector<std::string> expected{"a", "b", "c"};

		ASSERT_EQ(expected, ctx.getGlobal<std::vector<std::string>>("lines"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

TEST(TestJsFile, methodReadlineClosed)
{
	js::Context ctx;

	loadJsIrccd(ctx);
	loadJsFile(ctx);

	try {
		ctx.putGlobal("directory", IRCCD_TESTS_DIRECTORY);
		ctx.peval(js::Script{
			"lines = [];"
			"f = new Irccd.File(directory + '/lines.txt', 'r');"
			"f.close();"
			"for (var s; s = f.readline(); ) {"
			"  lines.push(s);"
			"}"
		});

		std::vector<std::string> expected;

		ASSERT_EQ(expected, ctx.getGlobal<std::vector<std::string>>("lines"));
	} catch (const std::exception &ex) {
		FAIL() << ex.what();
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
