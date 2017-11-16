/*
 * main.cpp -- test util functions
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "util"
#include <boost/test/unit_test.hpp>

#include <cstdint>

#include <irccd/util.hpp>
#include <irccd/fs_util.hpp>
#include <irccd/string_util.hpp>
#include <irccd/system.hpp>

namespace std {

std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& list)
{
    for (const auto& s : list)
        out << s << " ";

    return out;
}

} // !std

namespace irccd {

BOOST_AUTO_TEST_SUITE(format)

/*
 * string_util::format function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_CASE(nothing)
{
    std::string expected = "hello world!";
    std::string result = string_util::format("hello world!");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(escape)
{
    string_util::subst params;

    params.keywords.emplace("target", "hello");

    BOOST_REQUIRE_EQUAL("$@#", string_util::format("$@#"));
    BOOST_REQUIRE_EQUAL(" $ @ # ", string_util::format(" $ @ # "));
    BOOST_REQUIRE_EQUAL("#", string_util::format("#"));
    BOOST_REQUIRE_EQUAL(" # ", string_util::format(" # "));
    BOOST_REQUIRE_EQUAL("#@", string_util::format("#@"));
    BOOST_REQUIRE_EQUAL("##", string_util::format("##"));
    BOOST_REQUIRE_EQUAL("#!", string_util::format("#!"));
    BOOST_REQUIRE_EQUAL("#{target}", string_util::format("##{target}"));
    BOOST_REQUIRE_EQUAL("@hello", string_util::format("@#{target}", params));
    BOOST_REQUIRE_EQUAL("hello#", string_util::format("#{target}#", params));
    BOOST_REQUIRE_THROW(string_util::format("#{failure"), std::exception);
}

BOOST_AUTO_TEST_CASE(disable_date)
{
    string_util::subst params;

    params.flags &= ~(string_util::subst_flags::date);

    BOOST_REQUIRE_EQUAL("%H:%M", string_util::format("%H:%M", params));
}

BOOST_AUTO_TEST_CASE(disable_keywords)
{
    string_util::subst params;

    params.keywords.emplace("target", "hello");
    params.flags &= ~(string_util::subst_flags::keywords);

    BOOST_REQUIRE_EQUAL("#{target}", string_util::format("#{target}", params));
}

BOOST_AUTO_TEST_CASE(disable_env)
{
    string_util::subst params;

    params.flags &= ~(string_util::subst_flags::env);

    BOOST_REQUIRE_EQUAL("${HOME}", string_util::format("${HOME}", params));
}

BOOST_AUTO_TEST_CASE(keyword_simple)
{
    string_util::subst params;

    params.keywords.insert({"target", "irccd"});

    std::string expected = "hello irccd!";
    std::string result = string_util::format("hello #{target}!", params);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(keyword_multiple)
{
    string_util::subst params;

    params.keywords.insert({"target", "irccd"});
    params.keywords.insert({"source", "nightmare"});

    std::string expected = "hello irccd from nightmare!";
    std::string result = string_util::format("hello #{target} from #{source}!", params);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(keyword_adj_twice)
{
    string_util::subst params;

    params.keywords.insert({"target", "irccd"});

    std::string expected = "hello irccdirccd!";
    std::string result = string_util::format("hello #{target}#{target}!", params);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(keyword_missing)
{
    std::string expected = "hello !";
    std::string result = string_util::format("hello #{target}!");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(env_simple)
{
    std::string home = sys::env("HOME");

    if (!home.empty()) {
        std::string expected = "my home is " + home;
        std::string result = string_util::format("my home is ${HOME}");

        BOOST_REQUIRE_EQUAL(expected, result);
    }
}

BOOST_AUTO_TEST_CASE(env_missing)
{
    std::string expected = "value is ";
    std::string result = string_util::format("value is ${HOPE_THIS_VAR_NOT_EXIST}");

    BOOST_REQUIRE_EQUAL(expected, result);
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

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(cut)
{
    list expected { "msg", "#staff", "foo bar baz" };
    list result = string_util::split("msg;#staff;foo bar baz", ";", 3);

    BOOST_REQUIRE_EQUAL(expected, result);
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

    BOOST_REQUIRE_EQUAL("123", result);
}

BOOST_AUTO_TEST_CASE(right)
{
    std::string value = "123   ";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("123", result);
}

BOOST_AUTO_TEST_CASE(both)
{
    std::string value = "   123   ";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("123", result);
}

BOOST_AUTO_TEST_CASE(none)
{
    std::string value = "without";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("without", result);
}

BOOST_AUTO_TEST_CASE(betweenEmpty)
{
    std::string value = "one list";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("one list", result);
}

BOOST_AUTO_TEST_CASE(betweenLeft)
{
    std::string value = "  space at left";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("space at left", result);
}

BOOST_AUTO_TEST_CASE(betweenRight)
{
    std::string value = "space at right  ";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("space at right", result);
}

BOOST_AUTO_TEST_CASE(betweenBoth)
{
    std::string value = "  space at both  ";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("space at both", result);
}

BOOST_AUTO_TEST_CASE(empty)
{
    std::string value = "    ";
    std::string result = string_util::strip(value);

    BOOST_REQUIRE_EQUAL("", result);
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

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(one)
{
    std::string expected = "1";
    std::string result = string_util::join({1});

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(two)
{
    std::string expected = "1:2";
    std::string result = string_util::join({1, 2});

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(delimiterString)
{
    std::string expected = "1;;2;;3";
    std::string result = string_util::join({1, 2, 3}, ";;");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(delimiterChar)
{
    std::string expected = "1@2@3@4";
    std::string result = string_util::join({1, 2, 3, 4}, '@');

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::is_identifier function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_identifier_valid)

BOOST_AUTO_TEST_CASE(correct)
{
    BOOST_REQUIRE(string_util::is_identifier("localhost"));
    BOOST_REQUIRE(string_util::is_identifier("localhost2"));
    BOOST_REQUIRE(string_util::is_identifier("localhost2-4_"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    BOOST_REQUIRE(!string_util::is_identifier(""));
    BOOST_REQUIRE(!string_util::is_identifier("localhost with spaces"));
    BOOST_REQUIRE(!string_util::is_identifier("localhost*"));
    BOOST_REQUIRE(!string_util::is_identifier("&&"));
    BOOST_REQUIRE(!string_util::is_identifier("@'"));
    BOOST_REQUIRE(!string_util::is_identifier("##"));
    BOOST_REQUIRE(!string_util::is_identifier("===++"));
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
    BOOST_REQUIRE(string_util::is_boolean("true"));
    BOOST_REQUIRE(string_util::is_boolean("True"));
    BOOST_REQUIRE(string_util::is_boolean("TRUE"));
    BOOST_REQUIRE(string_util::is_boolean("TruE"));

    // yes
    BOOST_REQUIRE(string_util::is_boolean("yes"));
    BOOST_REQUIRE(string_util::is_boolean("Yes"));
    BOOST_REQUIRE(string_util::is_boolean("YES"));
    BOOST_REQUIRE(string_util::is_boolean("YeS"));

    // on
    BOOST_REQUIRE(string_util::is_boolean("on"));
    BOOST_REQUIRE(string_util::is_boolean("On"));
    BOOST_REQUIRE(string_util::is_boolean("oN"));
    BOOST_REQUIRE(string_util::is_boolean("ON"));

    // 1
    BOOST_REQUIRE(string_util::is_boolean("1"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    BOOST_REQUIRE(!string_util::is_boolean("false"));
    BOOST_REQUIRE(!string_util::is_boolean("lol"));
    BOOST_REQUIRE(!string_util::is_boolean(""));
    BOOST_REQUIRE(!string_util::is_boolean("0"));
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::is_number function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_number)

BOOST_AUTO_TEST_CASE(correct)
{
    BOOST_REQUIRE(string_util::is_number("123"));
    BOOST_REQUIRE(string_util::is_number("-123"));
    BOOST_REQUIRE(string_util::is_number("123.67"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    BOOST_REQUIRE(!string_util::is_number("lol"));
    BOOST_REQUIRE(!string_util::is_number("this is not a number"));
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * string_util::to_int function
 * ------------------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(to_int)

BOOST_AUTO_TEST_CASE(signed_to_int)
{
    BOOST_TEST(string_util::to_int("10")                     == 10);
    BOOST_TEST(string_util::to_int<std::int8_t>("-10")       == -10);
    BOOST_TEST(string_util::to_int<std::int8_t>("10")        == 10);
    BOOST_TEST(string_util::to_int<std::int16_t>("-1000")    == -1000);
    BOOST_TEST(string_util::to_int<std::int16_t>("1000")     == 1000);
    BOOST_TEST(string_util::to_int<std::int32_t>("-1000")    == -1000);
    BOOST_TEST(string_util::to_int<std::int32_t>("1000")     == 1000);
}

BOOST_AUTO_TEST_CASE(signed_to_int64)
{
    BOOST_TEST(string_util::to_int<std::int64_t>("-9223372036854775807") == -9223372036854775807LL);
    BOOST_TEST(string_util::to_int<std::int64_t>("9223372036854775807") == 9223372036854775807LL);
}

BOOST_AUTO_TEST_CASE(unsigned_to_uint)
{
    BOOST_TEST(string_util::to_uint("10")                    == 10U);
    BOOST_TEST(string_util::to_uint<std::uint8_t>("10")       == 10U);
    BOOST_TEST(string_util::to_uint<std::uint16_t>("1000")    == 1000U);
    BOOST_TEST(string_util::to_uint<std::uint32_t>("1000")    == 1000U);
}

BOOST_AUTO_TEST_CASE(unsigned_to_uint64)
{
    BOOST_TEST(string_util::to_uint<std::uint64_t>("18446744073709551615") == 18446744073709551615ULL);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(errors)

BOOST_AUTO_TEST_CASE(invalid_argument)
{
    BOOST_REQUIRE_THROW(string_util::to_int("plopation"), std::invalid_argument);
    BOOST_REQUIRE_THROW(string_util::to_uint("plopation"), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(out_of_range)
{
    BOOST_REQUIRE_THROW(string_util::to_int<std::int8_t>("1000"), std::out_of_range);
    BOOST_REQUIRE_THROW(string_util::to_int<std::int8_t>("-1000"), std::out_of_range);
    BOOST_REQUIRE_THROW(string_util::to_uint<std::uint8_t>("1000"), std::out_of_range);
    BOOST_REQUIRE_THROW(string_util::to_uint<std::uint8_t>("-1000"), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * fs_util::find function (name)
 * ------------------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(fs_find_name)

BOOST_AUTO_TEST_CASE(not_recursive)
{
    auto file1 = fs_util::find(TESTS_BINARY_DIR "/root", "file-1.txt", false);
    auto file2 = fs_util::find(TESTS_BINARY_DIR "/root", "file-2.txt", false);

    BOOST_TEST(file1.find("file-1.txt") != std::string::npos);
    BOOST_TEST(file2.empty());
}

BOOST_AUTO_TEST_CASE(recursive)
{
    auto file1 = fs_util::find(TESTS_BINARY_DIR "/root", "file-1.txt", true);
    auto file2 = fs_util::find(TESTS_BINARY_DIR "/root", "file-2.txt", true);

    BOOST_TEST(file1.find("file-1.txt") != std::string::npos);
    BOOST_TEST(file2.find("file-2.txt") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * fs_util::find function (regex)
 * ------------------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(fs_find_regex)

BOOST_AUTO_TEST_CASE(not_recursive)
{
    const std::regex regex("file-[12]\\.txt");

    auto file = fs_util::find(TESTS_BINARY_DIR "/root", regex, false);

    BOOST_TEST(file.find("file-1.txt") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(recursive)
{
    const std::regex regex("file-[12]\\.txt");

    auto file = fs_util::find(TESTS_BINARY_DIR "/root/level-a", regex, true);

    BOOST_TEST(file.find("file-2.txt") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
