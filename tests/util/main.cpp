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
 * util::format function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_CASE(nothing)
{
    std::string expected = "hello world!";
    std::string result = util::format("hello world!");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(escape)
{
    util::subst params;

    params.keywords.emplace("target", "hello");

    BOOST_REQUIRE_EQUAL("$@#", util::format("$@#"));
    BOOST_REQUIRE_EQUAL(" $ @ # ", util::format(" $ @ # "));
    BOOST_REQUIRE_EQUAL("#", util::format("#"));
    BOOST_REQUIRE_EQUAL(" # ", util::format(" # "));
    BOOST_REQUIRE_EQUAL("#@", util::format("#@"));
    BOOST_REQUIRE_EQUAL("##", util::format("##"));
    BOOST_REQUIRE_EQUAL("#!", util::format("#!"));
    BOOST_REQUIRE_EQUAL("#{target}", util::format("##{target}"));
    BOOST_REQUIRE_EQUAL("@hello", util::format("@#{target}", params));
    BOOST_REQUIRE_EQUAL("hello#", util::format("#{target}#", params));
    BOOST_REQUIRE_THROW(util::format("#{failure"), std::exception);
}

BOOST_AUTO_TEST_CASE(disable_date)
{
    util::subst params;

    params.flags &= ~(util::subst_flags::date);

    BOOST_REQUIRE_EQUAL("%H:%M", util::format("%H:%M", params));
}

BOOST_AUTO_TEST_CASE(disable_keywords)
{
    util::subst params;

    params.keywords.emplace("target", "hello");
    params.flags &= ~(util::subst_flags::keywords);

    BOOST_REQUIRE_EQUAL("#{target}", util::format("#{target}", params));
}

BOOST_AUTO_TEST_CASE(disable_env)
{
    util::subst params;

    params.flags &= ~(util::subst_flags::env);

    BOOST_REQUIRE_EQUAL("${HOME}", util::format("${HOME}", params));
}

BOOST_AUTO_TEST_CASE(keyword_simple)
{
    util::subst params;

    params.keywords.insert({"target", "irccd"});

    std::string expected = "hello irccd!";
    std::string result = util::format("hello #{target}!", params);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(keyword_multiple)
{
    util::subst params;

    params.keywords.insert({"target", "irccd"});
    params.keywords.insert({"source", "nightmare"});

    std::string expected = "hello irccd from nightmare!";
    std::string result = util::format("hello #{target} from #{source}!", params);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(keyword_adj_twice)
{
    util::subst params;

    params.keywords.insert({"target", "irccd"});

    std::string expected = "hello irccdirccd!";
    std::string result = util::format("hello #{target}#{target}!", params);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(keyword_missing)
{
    std::string expected = "hello !";
    std::string result = util::format("hello #{target}!");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(env_simple)
{
    std::string home = sys::env("HOME");

    if (!home.empty()) {
        std::string expected = "my home is " + home;
        std::string result = util::format("my home is ${HOME}");

        BOOST_REQUIRE_EQUAL(expected, result);
    }
}

BOOST_AUTO_TEST_CASE(env_missing)
{
    std::string expected = "value is ";
    std::string result = util::format("value is ${HOPE_THIS_VAR_NOT_EXIST}");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::split function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(split)

using list = std::vector<std::string>;

BOOST_AUTO_TEST_CASE(simple)
{
    list expected { "a", "b" };
    list result = util::split("a;b", ";");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(cut)
{
    list expected { "msg", "#staff", "foo bar baz" };
    list result = util::split("msg;#staff;foo bar baz", ";", 3);

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::strip function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(strip)

BOOST_AUTO_TEST_CASE(left)
{
    std::string value = "   123";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("123", result);
}

BOOST_AUTO_TEST_CASE(right)
{
    std::string value = "123   ";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("123", result);
}

BOOST_AUTO_TEST_CASE(both)
{
    std::string value = "   123   ";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("123", result);
}

BOOST_AUTO_TEST_CASE(none)
{
    std::string value = "without";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("without", result);
}

BOOST_AUTO_TEST_CASE(betweenEmpty)
{
    std::string value = "one list";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("one list", result);
}

BOOST_AUTO_TEST_CASE(betweenLeft)
{
    std::string value = "  space at left";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("space at left", result);
}

BOOST_AUTO_TEST_CASE(betweenRight)
{
    std::string value = "space at right  ";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("space at right", result);
}

BOOST_AUTO_TEST_CASE(betweenBoth)
{
    std::string value = "  space at both  ";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("space at both", result);
}

BOOST_AUTO_TEST_CASE(empty)
{
    std::string value = "    ";
    std::string result = util::strip(value);

    BOOST_REQUIRE_EQUAL("", result);
} 

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::join function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(join)

BOOST_AUTO_TEST_CASE(empty)
{
    std::string expected = "";
    std::string result = util::join<int>({});

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(one)
{
    std::string expected = "1";
    std::string result = util::join({1});

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(two)
{
    std::string expected = "1:2";
    std::string result = util::join({1, 2});

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(delimiterString)
{
    std::string expected = "1;;2;;3";
    std::string result = util::join({1, 2, 3}, ";;");

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_CASE(delimiterChar)
{
    std::string expected = "1@2@3@4";
    std::string result = util::join({1, 2, 3, 4}, '@');

    BOOST_REQUIRE_EQUAL(expected, result);
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::is_identifier function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_identifier_valid)

BOOST_AUTO_TEST_CASE(correct)
{
    BOOST_REQUIRE(util::is_identifier("localhost"));
    BOOST_REQUIRE(util::is_identifier("localhost2"));
    BOOST_REQUIRE(util::is_identifier("localhost2-4_"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    BOOST_REQUIRE(!util::is_identifier(""));
    BOOST_REQUIRE(!util::is_identifier("localhost with spaces"));
    BOOST_REQUIRE(!util::is_identifier("localhost*"));
    BOOST_REQUIRE(!util::is_identifier("&&"));
    BOOST_REQUIRE(!util::is_identifier("@'"));
    BOOST_REQUIRE(!util::is_identifier("##"));
    BOOST_REQUIRE(!util::is_identifier("===++"));
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::is_boolean function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_boolean)

BOOST_AUTO_TEST_CASE(correct)
{
    // true
    BOOST_REQUIRE(util::is_boolean("true"));
    BOOST_REQUIRE(util::is_boolean("True"));
    BOOST_REQUIRE(util::is_boolean("TRUE"));
    BOOST_REQUIRE(util::is_boolean("TruE"));

    // yes
    BOOST_REQUIRE(util::is_boolean("yes"));
    BOOST_REQUIRE(util::is_boolean("Yes"));
    BOOST_REQUIRE(util::is_boolean("YES"));
    BOOST_REQUIRE(util::is_boolean("YeS"));

    // on
    BOOST_REQUIRE(util::is_boolean("on"));
    BOOST_REQUIRE(util::is_boolean("On"));
    BOOST_REQUIRE(util::is_boolean("oN"));
    BOOST_REQUIRE(util::is_boolean("ON"));

    // 1
    BOOST_REQUIRE(util::is_boolean("1"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    BOOST_REQUIRE(!util::is_boolean("false"));
    BOOST_REQUIRE(!util::is_boolean("lol"));
    BOOST_REQUIRE(!util::is_boolean(""));
    BOOST_REQUIRE(!util::is_boolean("0"));
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::is_number function
 * --------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(is_number)

BOOST_AUTO_TEST_CASE(correct)
{
    BOOST_REQUIRE(util::is_number("123"));
    BOOST_REQUIRE(util::is_number("-123"));
    BOOST_REQUIRE(util::is_number("123.67"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    BOOST_REQUIRE(!util::is_number("lol"));
    BOOST_REQUIRE(!util::is_number("this is not a number"));
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * util::to_number function
 * ------------------------------------------------------------------
 */

BOOST_AUTO_TEST_SUITE(to_number)

BOOST_AUTO_TEST_CASE(correct)
{
    /* unsigned */
    BOOST_REQUIRE_EQUAL(50u, util::to_number<std::uint8_t>("50"));
    BOOST_REQUIRE_EQUAL(5000u, util::to_number<std::uint16_t>("5000"));
    BOOST_REQUIRE_EQUAL(50000u, util::to_number<std::uint32_t>("50000"));
    BOOST_REQUIRE_EQUAL(500000u, util::to_number<std::uint64_t>("500000"));

    /* signed */
    BOOST_REQUIRE_EQUAL(-50, util::to_number<std::int8_t>("-50"));
    BOOST_REQUIRE_EQUAL(-500, util::to_number<std::int16_t>("-500"));
    BOOST_REQUIRE_EQUAL(-5000, util::to_number<std::int32_t>("-5000"));
    BOOST_REQUIRE_EQUAL(-50000, util::to_number<std::int64_t>("-50000"));
}

BOOST_AUTO_TEST_CASE(incorrect)
{
    /* unsigned */
    BOOST_REQUIRE_THROW(util::to_number<std::uint8_t>("300"), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::uint16_t>("80000"), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::uint8_t>("-125"), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::uint16_t>("-25000"), std::out_of_range);

    /* signed */
    BOOST_REQUIRE_THROW(util::to_number<std::int8_t>("300"), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::int16_t>("80000"), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::int8_t>("-300"), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::int16_t>("-80000"), std::out_of_range);

    /* not numbers */
    BOOST_REQUIRE_THROW(util::to_number<std::uint8_t>("nonono"), std::invalid_argument);

    /* custom ranges */
    BOOST_REQUIRE_THROW(util::to_number<std::uint8_t>("50", 0, 10), std::out_of_range);
    BOOST_REQUIRE_THROW(util::to_number<std::int8_t>("-50", -10, 10), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
