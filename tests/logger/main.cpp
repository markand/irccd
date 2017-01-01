/*
 * main.cpp -- test logger functions
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

#include <algorithm>

#include <gtest/gtest.h>

#include <irccd/logger.hpp>

using namespace irccd;

namespace {

std::string lineDebug;
std::string lineInfo;
std::string lineWarning;

} // !namespace

class MyInterface : public log::Logger {
public:
    void debug(const std::string &line) override
    {
        lineDebug = line;
    }

    void info(const std::string &line) override
    {
        lineInfo = line;
    }

    void warning(const std::string &line) override
    {
        lineWarning = line;
    }
};

class MyFilter : public log::Filter {
public:
    std::string preDebug(std::string input) const override
    {
        return std::reverse(input.begin(), input.end()), input;
    }

    std::string preInfo(std::string input) const override
    {
        return std::reverse(input.begin(), input.end()), input;
    }

    std::string preWarning(std::string input) const override
    {
        return std::reverse(input.begin(), input.end()), input;
    }
};

#if !defined(NDEBUG)

TEST(Logger, debug)
{
    log::debug("debug");

    ASSERT_EQ("gubed", lineDebug);
}

#endif

TEST(Logger, info)
{
    log::info("info");

    ASSERT_EQ("ofni", lineInfo);
}

TEST(Logger, warning)
{
    log::warning("warning");

    ASSERT_EQ("gninraw", lineWarning);
}

int main(int argc, char **argv)
{
    log::setVerbose(true);
    log::setLogger(std::make_unique<MyInterface>());
    log::setFilter(std::make_unique<MyFilter>());

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
