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

#include <cstdint>

#include <gtest/gtest.h>

#include <irccd/util.hpp>
#include <irccd/system.hpp>

namespace irccd {

/* --------------------------------------------------------
 * util::format function
 * -------------------------------------------------------- */

TEST(Format, nothing)
{
    std::string expected = "hello world!";
    std::string result = util::format("hello world!");

    ASSERT_EQ(expected, result);
}

TEST(Format, escape)
{
    util::Substitution params;

    params.keywords.emplace("target", "hello");

    ASSERT_EQ("$@#", util::format("$@#"));
    ASSERT_EQ(" $ @ # ", util::format(" $ @ # "));
    ASSERT_EQ("#", util::format("#"));
    ASSERT_EQ(" # ", util::format(" # "));
    ASSERT_EQ("#@", util::format("#@"));
    ASSERT_EQ("##", util::format("##"));
    ASSERT_EQ("#!", util::format("#!"));
    ASSERT_EQ("#{target}", util::format("##{target}"));
    ASSERT_EQ("@hello", util::format("@#{target}", params));
    ASSERT_EQ("hello#", util::format("#{target}#", params));
    ASSERT_ANY_THROW(util::format("#{failure"));
}

TEST(Format, disableDate)
{
    util::Substitution params;

    params.flags &= ~(util::Substitution::Date);

    ASSERT_EQ("%H:%M", util::format("%H:%M", params));
}

TEST(Format, disableKeywords)
{
    util::Substitution params;

    params.keywords.emplace("target", "hello");
    params.flags &= ~(util::Substitution::Keywords);

    ASSERT_EQ("#{target}", util::format("#{target}", params));
}

TEST(Format, disableEnv)
{
    util::Substitution params;

    params.flags &= ~(util::Substitution::Env);

    ASSERT_EQ("${HOME}", util::format("${HOME}", params));
}

TEST(Format, keywordSimple)
{
    util::Substitution params;

    params.keywords.insert({"target", "irccd"});

    std::string expected = "hello irccd!";
    std::string result = util::format("hello #{target}!", params);

    ASSERT_EQ(expected, result);
}

TEST(Format, keywordMultiple)
{
    util::Substitution params;

    params.keywords.insert({"target", "irccd"});
    params.keywords.insert({"source", "nightmare"});

    std::string expected = "hello irccd from nightmare!";
    std::string result = util::format("hello #{target} from #{source}!", params);

    ASSERT_EQ(expected, result);
}

TEST(Format, keywordAdjTwice)
{
    util::Substitution params;

    params.keywords.insert({"target", "irccd"});

    std::string expected = "hello irccdirccd!";
    std::string result = util::format("hello #{target}#{target}!", params);

    ASSERT_EQ(expected, result);
}

TEST(Format, keywordMissing)
{
    std::string expected = "hello !";
    std::string result = util::format("hello #{target}!");

    ASSERT_EQ(expected, result);
}

TEST(Format, envSimple)
{
    std::string home = sys::env("HOME");

    if (!home.empty()) {
        std::string expected = "my home is " + home;
        std::string result = util::format("my home is ${HOME}");

        ASSERT_EQ(expected, result);
    }
}

TEST(Format, envMissing)
{
    std::string expected = "value is ";
    std::string result = util::format("value is ${HOPE_THIS_VAR_NOT_EXIST}");

    ASSERT_EQ(expected, result);
}

/* --------------------------------------------------------
 * util::split function
 * -------------------------------------------------------- */

using List = std::vector<std::string>;

TEST(Split, simple)
{
    List expected { "a", "b" };
    List result = util::split("a;b", ";");

    ASSERT_EQ(expected, result);
}

TEST(Split, cut)
{
    List expected { "msg", "#staff", "foo bar baz" };
    List result = util::split("msg;#staff;foo bar baz", ";", 3);

    ASSERT_EQ(expected, result);
}

/* --------------------------------------------------------
 * util::strip function
 * -------------------------------------------------------- */

TEST(Strip, left)
{
    std::string value = "   123";
    std::string result = util::strip(value);

    ASSERT_EQ("123", result);
}

TEST(Strip, right)
{
    std::string value = "123   ";
    std::string result = util::strip(value);

    ASSERT_EQ("123", result);
}

TEST(Strip, both)
{
    std::string value = "   123   ";
    std::string result = util::strip(value);

    ASSERT_EQ("123", result);
}

TEST(Strip, none)
{
    std::string value = "without";
    std::string result = util::strip(value);

    ASSERT_EQ("without", result);
}

TEST(Strip, betweenEmpty)
{
    std::string value = "one list";
    std::string result = util::strip(value);

    ASSERT_EQ("one list", result);
}

TEST(Strip, betweenLeft)
{
    std::string value = "  space at left";
    std::string result = util::strip(value);

    ASSERT_EQ("space at left", result);
}

TEST(Strip, betweenRight)
{
    std::string value = "space at right  ";
    std::string result = util::strip(value);

    ASSERT_EQ("space at right", result);
}

TEST(Strip, betweenBoth)
{
    std::string value = "  space at both  ";
    std::string result = util::strip(value);

    ASSERT_EQ("space at both", result);
}

TEST(Strip, empty)
{
    std::string value = "    ";
    std::string result = util::strip(value);

    ASSERT_EQ("", result);
}

/* --------------------------------------------------------
 * util::join function
 * -------------------------------------------------------- */

TEST(Join, empty)
{
    std::string expected = "";
    std::string result = util::join<int>({});

    ASSERT_EQ(expected, result);
}

TEST(Join, one)
{
    std::string expected = "1";
    std::string result = util::join({1});

    ASSERT_EQ(expected, result);
}

TEST(Join, two)
{
    std::string expected = "1:2";
    std::string result = util::join({1, 2});

    ASSERT_EQ(expected, result);
}

TEST(Join, delimiterString)
{
    std::string expected = "1;;2;;3";
    std::string result = util::join({1, 2, 3}, ";;");

    ASSERT_EQ(expected, result);
}

TEST(Join, delimiterChar)
{
    std::string expected = "1@2@3@4";
    std::string result = util::join({1, 2, 3, 4}, '@');

    ASSERT_EQ(expected, result);
}

/* --------------------------------------------------------
 * util::isIdentifierValid function
 * -------------------------------------------------------- */

TEST(IsIdentifierValid, correct)
{
    ASSERT_TRUE(util::isIdentifierValid("localhost"));
    ASSERT_TRUE(util::isIdentifierValid("localhost2"));
    ASSERT_TRUE(util::isIdentifierValid("localhost2-4_"));
}

TEST(IsIdentifierValid, incorrect)
{
    ASSERT_FALSE(util::isIdentifierValid(""));
    ASSERT_FALSE(util::isIdentifierValid("localhost with spaces"));
    ASSERT_FALSE(util::isIdentifierValid("localhost*"));
    ASSERT_FALSE(util::isIdentifierValid("&&"));
    ASSERT_FALSE(util::isIdentifierValid("@'"));
    ASSERT_FALSE(util::isIdentifierValid("##"));
    ASSERT_FALSE(util::isIdentifierValid("===++"));
}

/* --------------------------------------------------------
 * util::isBoolean function
 * -------------------------------------------------------- */

TEST(IsBoolean, correct)
{
    // true
    ASSERT_TRUE(util::isBoolean("true"));
    ASSERT_TRUE(util::isBoolean("True"));
    ASSERT_TRUE(util::isBoolean("TRUE"));
    ASSERT_TRUE(util::isBoolean("TruE"));

    // yes
    ASSERT_TRUE(util::isBoolean("yes"));
    ASSERT_TRUE(util::isBoolean("Yes"));
    ASSERT_TRUE(util::isBoolean("YES"));
    ASSERT_TRUE(util::isBoolean("YeS"));

    // on
    ASSERT_TRUE(util::isBoolean("on"));
    ASSERT_TRUE(util::isBoolean("On"));
    ASSERT_TRUE(util::isBoolean("oN"));
    ASSERT_TRUE(util::isBoolean("ON"));

    // 1
    ASSERT_TRUE(util::isBoolean("1"));
}

TEST(IsBoolean, incorrect)
{
    ASSERT_FALSE(util::isBoolean("false"));
    ASSERT_FALSE(util::isBoolean("lol"));
    ASSERT_FALSE(util::isBoolean(""));
    ASSERT_FALSE(util::isBoolean("0"));
}

/* --------------------------------------------------------
 * util::isNumber function
 * -------------------------------------------------------- */

TEST(IsNumber, correct)
{
    ASSERT_TRUE(util::isNumber("123"));
    ASSERT_TRUE(util::isNumber("-123"));
    ASSERT_TRUE(util::isNumber("123.67"));
}

TEST(IsNumber, incorrect)
{
    ASSERT_FALSE(util::isNumber("lol"));
    ASSERT_FALSE(util::isNumber("this is not a number"));
}

/*
 * util::toNumber function
 * ------------------------------------------------------------------
 */

TEST(ToNumber, correct)
{
    /* unsigned */
    ASSERT_EQ(50u, util::toNumber<std::uint8_t>("50"));
    ASSERT_EQ(5000u, util::toNumber<std::uint16_t>("5000"));
    ASSERT_EQ(50000u, util::toNumber<std::uint32_t>("50000"));
    ASSERT_EQ(500000u, util::toNumber<std::uint64_t>("500000"));

    /* signed */
    ASSERT_EQ(-50, util::toNumber<std::int8_t>("-50"));
    ASSERT_EQ(-500, util::toNumber<std::int16_t>("-500"));
    ASSERT_EQ(-5000, util::toNumber<std::int32_t>("-5000"));
    ASSERT_EQ(-50000, util::toNumber<std::int64_t>("-50000"));
}

TEST(ToNumber, incorrect)
{
    /* unsigned */
    ASSERT_THROW(util::toNumber<std::uint8_t>("300"), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::uint16_t>("80000"), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::uint8_t>("-125"), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::uint16_t>("-25000"), std::out_of_range);

    /* signed */
    ASSERT_THROW(util::toNumber<std::int8_t>("300"), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::int16_t>("80000"), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::int8_t>("-300"), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::int16_t>("-80000"), std::out_of_range);

    /* not numbers */
    ASSERT_THROW(util::toNumber<std::uint8_t>("nonono"), std::invalid_argument);

    /* custom ranges */
    ASSERT_THROW(util::toNumber<std::uint8_t>("50", 0, 10), std::out_of_range);
    ASSERT_THROW(util::toNumber<std::int8_t>("-50", -10, 10), std::out_of_range);
}

} // !irccd

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
