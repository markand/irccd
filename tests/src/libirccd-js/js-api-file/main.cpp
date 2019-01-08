/*
 * main.cpp -- test Irccd.File API
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

#define BOOST_TEST_MODULE "File Javascript API"
#include <boost/test/unit_test.hpp>

#include <fstream>

#include <irccd/test/js_fixture.hpp>

using namespace irccd::js;
using namespace irccd::test;

namespace irccd {

namespace {

class file_js_fixture : public js_fixture {
public:
	file_js_fixture()
	{
		duk::push(plugin_->get_context(), CMAKE_SOURCE_DIR);
		duk_put_global_string(plugin_->get_context(), "CMAKE_SOURCE_DIR");
	}
};

BOOST_FIXTURE_TEST_SUITE(file_js_api_suite, file_js_fixture)

BOOST_AUTO_TEST_CASE(function_basename)
{
	if (duk_peval_string(plugin_->get_context(), "result = Irccd.File.basename('/usr/local/etc/irccd.conf');"))
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("irccd.conf" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(function_dirname)
{
	if (duk_peval_string(plugin_->get_context(), "result = Irccd.File.dirname('/usr/local/etc/irccd.conf');"))
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("/usr/local/etc" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(function_exists)
{
	if (duk_peval_string(plugin_->get_context(), "result = Irccd.File.exists(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt')"))
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(function_exists2)
{
	if (duk_peval_string(plugin_->get_context(), "result = Irccd.File.exists('file_which_does_not_exist.txt')"))
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(!duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(function_remove)
{
	// First create a dummy file
	std::ofstream("test-js-fs.remove");

	if (duk_peval_string(plugin_->get_context(), "Irccd.File.remove('test-js-fs.remove');") != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	std::ifstream in("test-js-fs.remove");

	BOOST_TEST(!in.is_open());
}

BOOST_AUTO_TEST_CASE(method_basename)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"result = f.basename();"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("file-1.txt" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_basename_closed)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"result = f.basename();"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("file-1.txt" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_dirname)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"result = f.dirname();"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(CMAKE_SOURCE_DIR "/tests/data/root" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_dirname_closed)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"result = f.dirname();"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(CMAKE_SOURCE_DIR "/tests/data/root" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_lines)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"result = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/lines.txt', 'r').lines();"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	std::vector<std::string> expected{"a", "b", "c"};

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(expected == duk::get<std::vector<std::string>>(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek1)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekSet, 6);"
		"result = f.read(1);"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(".", duk::get<std::string>(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek1_closed)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"f.seek(Irccd.File.SeekSet, 4);"
		"result = f.read(1);"
		"result = typeof (result) === \"undefined\";"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek2)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekSet, 2);"
		"f.seek(Irccd.File.SeekCur, 4);"
		"result = f.read(1);"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("." == duk::get<std::string>(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek2c_losed)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"f.seek(Irccd.File.SeekSet, 2);"
		"f.seek(Irccd.File.SeekCur, 2);"
		"result = f.read(1);"
		"result = typeof (result) === \"undefined\";"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek3)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.seek(Irccd.File.SeekEnd, -2);"
		"result = f.read(1);"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("t" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_seek3_closed)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"f.close();"
		"f.seek(Irccd.File.SeekEnd, -2);"
		"result = f.read(1);"
		"result = typeof (result) === \"undefined\";"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(duk_get_boolean(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_read1)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/file-1.txt', 'r');"
		"result = f.read();"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST("file-1.txt\n" == duk_get_string(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_readline)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"result = [];"
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/lines.txt', 'r');"
		"for (var s; s = f.readline(); ) {"
		"  result.push(s);"
		"}"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	std::vector<std::string> expected{"a", "b", "c"};

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(expected == duk::get<std::vector<std::string>>(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_CASE(method_readline_closed)
{
	const auto ret = duk_peval_string(plugin_->get_context(),
		"result = [];"
		"f = new Irccd.File(CMAKE_SOURCE_DIR + '/tests/data/root/lines.txt', 'r');"
		"f.close();"
		"for (var s; s = f.readline(); ) {"
		"  result.push(s);"
		"}"
	);

	if (ret != 0)
		throw duk::get_stack(plugin_->get_context(), -1);

	std::vector<std::string> expected;

	BOOST_TEST(duk_get_global_string(plugin_->get_context(), "result"));
	BOOST_TEST(expected == duk::get<std::vector<std::string>>(plugin_->get_context(), -1));
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
