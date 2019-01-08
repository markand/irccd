/*
 * main.cpp -- test string_util functions
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

#define BOOST_TEST_MODULE "string_util"
#include <boost/test/unit_test.hpp>

#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

namespace irccd {

namespace {

/*
 * string_util::format function
 * --------------------------------------------------------
 */
BOOST_AUTO_TEST_SUITE(format)

BOOST_AUTO_TEST_CASE(nothing)
{
	std::string expected = "hello world!";
	std::string result = string_util::format("hello world!");

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(escape)
{
	string_util::subst params;

	params.keywords.emplace("target", "hello");

	BOOST_TEST(string_util::format("$@#") == "$@#");
	BOOST_TEST(string_util::format(" $ @ # ") == " $ @ # ");
	BOOST_TEST(string_util::format("#") == "#");
	BOOST_TEST(string_util::format(" # ") == " # ");
	BOOST_TEST(string_util::format("#@") == "#@");
	BOOST_TEST(string_util::format("##") == "##");
	BOOST_TEST(string_util::format("#!") == "#!");
	BOOST_TEST(string_util::format("##{target}") == "#{target}");
	BOOST_TEST(string_util::format("@#{target}", params) == "@hello");
	BOOST_TEST(string_util::format("#{target}#", params) == "hello#");
	BOOST_REQUIRE_THROW(string_util::format("#{failure"), std::exception);
}

BOOST_AUTO_TEST_CASE(disable_date)
{
	string_util::subst params;

	params.flags &= ~(string_util::subst_flags::date);

	BOOST_TEST(string_util::format("%H:%M", params) == "%H:%M");
}

BOOST_AUTO_TEST_CASE(disable_keywords)
{
	string_util::subst params;

	params.keywords.emplace("target", "hello");
	params.flags &= ~(string_util::subst_flags::keywords);

	BOOST_TEST(string_util::format("#{target}", params) == "#{target}");
}

BOOST_AUTO_TEST_CASE(disable_env)
{
	string_util::subst params;

	params.flags &= ~(string_util::subst_flags::env);

	BOOST_TEST(string_util::format("${HOME}", params) == "${HOME}");
}

BOOST_AUTO_TEST_CASE(keyword_simple)
{
	string_util::subst params;

	params.keywords.insert({"target", "irccd"});

	std::string expected = "hello irccd!";
	std::string result = string_util::format("hello #{target}!", params);

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(keyword_multiple)
{
	string_util::subst params;

	params.keywords.insert({"target", "irccd"});
	params.keywords.insert({"source", "nightmare"});

	std::string expected = "hello irccd from nightmare!";
	std::string result = string_util::format("hello #{target} from #{source}!", params);

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(keyword_adj_twice)
{
	string_util::subst params;

	params.keywords.insert({"target", "irccd"});

	std::string expected = "hello irccdirccd!";
	std::string result = string_util::format("hello #{target}#{target}!", params);

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(keyword_missing)
{
	std::string expected = "hello !";
	std::string result = string_util::format("hello #{target}!");

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(env_simple)
{
	std::string home = sys::env("HOME");

	if (!home.empty()) {
		std::string expected = "my home is " + home;
		std::string result = string_util::format("my home is ${HOME}");

		BOOST_TEST(expected == result);
	}
}

BOOST_AUTO_TEST_CASE(env_missing)
{
	std::string expected = "value is ";
	std::string result = string_util::format("value is ${HOPE_THIS_VAR_NOT_EXIST}");

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::split function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(split)

using list = std::vector<std::string>;

BOOST_AUTO_TEST_CASE(simple)
{
	list expected { "a", "b" };
	list result = string_util::split("a;b", ";");

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(cut)
{
	list expected { "msg", "#staff", "foo bar baz" };
	list result = string_util::split("msg;#staff;foo bar baz", ";", 3);

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::strip function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(strip)

BOOST_AUTO_TEST_CASE(left)
{
	std::string value = "   123";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "123");
}

BOOST_AUTO_TEST_CASE(right)
{
	std::string value = "123   ";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "123");
}

BOOST_AUTO_TEST_CASE(both)
{
	std::string value = "   123   ";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "123");
}

BOOST_AUTO_TEST_CASE(none)
{
	std::string value = "without";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "without");
}

BOOST_AUTO_TEST_CASE(between_empty)
{
	std::string value = "one list";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "one list");
}

BOOST_AUTO_TEST_CASE(between_left)
{
	std::string value = "  space at left";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "space at left");
}

BOOST_AUTO_TEST_CASE(between_right)
{
	std::string value = "space at right  ";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "space at right");
}

BOOST_AUTO_TEST_CASE(between_both)
{
	std::string value = "  space at both  ";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "space at both");
}

BOOST_AUTO_TEST_CASE(empty)
{
	std::string value = "    ";
	std::string result = string_util::strip(value);

	BOOST_TEST(result == "");
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::join function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(join)

BOOST_AUTO_TEST_CASE(empty)
{
	std::string expected = "";
	std::string result = string_util::join<int>({});

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(one)
{
	std::string expected = "1";
	std::string result = string_util::join({1});

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(two)
{
	std::string expected = "1:2";
	std::string result = string_util::join({1, 2});

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(delimiter_string)
{
	std::string expected = "1;;2;;3";
	std::string result = string_util::join({1, 2, 3}, ";;");

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_CASE(delimiter_char)
{
	std::string expected = "1@2@3@4";
	std::string result = string_util::join({1, 2, 3, 4}, '@');

	BOOST_TEST(expected == result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::is_identifier function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_identifier_valid)

BOOST_AUTO_TEST_CASE(correct)
{
	BOOST_TEST(string_util::is_identifier("localhost"));
	BOOST_TEST(string_util::is_identifier("localhost2"));
	BOOST_TEST(string_util::is_identifier("localhost2-4_"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
	BOOST_TEST(!string_util::is_identifier(""));
	BOOST_TEST(!string_util::is_identifier("localhost with spaces"));
	BOOST_TEST(!string_util::is_identifier("localhost*"));
	BOOST_TEST(!string_util::is_identifier("&&"));
	BOOST_TEST(!string_util::is_identifier("@'"));
	BOOST_TEST(!string_util::is_identifier("##"));
	BOOST_TEST(!string_util::is_identifier("===++"));
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::is_boolean function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_boolean)

BOOST_AUTO_TEST_CASE(correct)
{
	// true
	BOOST_TEST(string_util::is_boolean("true"));
	BOOST_TEST(string_util::is_boolean("True"));
	BOOST_TEST(string_util::is_boolean("TRUE"));
	BOOST_TEST(string_util::is_boolean("TruE"));

	// yes
	BOOST_TEST(string_util::is_boolean("yes"));
	BOOST_TEST(string_util::is_boolean("Yes"));
	BOOST_TEST(string_util::is_boolean("YES"));
	BOOST_TEST(string_util::is_boolean("YeS"));

	// on
	BOOST_TEST(string_util::is_boolean("on"));
	BOOST_TEST(string_util::is_boolean("On"));
	BOOST_TEST(string_util::is_boolean("oN"));
	BOOST_TEST(string_util::is_boolean("ON"));

	// 1
	BOOST_TEST(string_util::is_boolean("1"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
	BOOST_TEST(!string_util::is_boolean("false"));
	BOOST_TEST(!string_util::is_boolean("lol"));
	BOOST_TEST(!string_util::is_boolean(""));
	BOOST_TEST(!string_util::is_boolean("0"));
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
