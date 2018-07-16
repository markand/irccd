/*
 * main.cpp -- test logger functions
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#define BOOST_TEST_MODULE "Logger"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/logger.hpp>

namespace irccd {

namespace {

class my_logger : public logger {
public:
    std::string line_debug;
    std::string line_info;
    std::string line_warning;

    void write_debug(const std::string& line) override
    {
        line_debug = line;
    }

    void write_info(const std::string& line) override
    {
        line_info = line;
    }

    void write_warning(const std::string& line) override
    {
        line_warning = line;
    }
};

class my_filter : public logger_filter {
public:
    std::string pre_debug(std::string input) const override
    {
        return std::reverse(input.begin(), input.end()), input;
    }

    std::string pre_info(std::string input) const override
    {
        return std::reverse(input.begin(), input.end()), input;
    }

    std::string pre_warning(std::string input) const override
    {
        return std::reverse(input.begin(), input.end()), input;
    }
};

class logger_test {
public:
    my_logger log_;

    logger_test()
    {
        log_.set_filter(std::make_unique<my_filter>());
        log_.set_verbose(true);
    }
};

BOOST_FIXTURE_TEST_SUITE(logger_test_suite, logger_test)

#if !defined(NDEBUG)

BOOST_AUTO_TEST_CASE(debug)
{
    log_.debug("debug");

    BOOST_REQUIRE_EQUAL("gubed", log_.line_debug);
}

#endif

BOOST_AUTO_TEST_CASE(info)
{
    log_.info("info");

    BOOST_REQUIRE_EQUAL("ofni", log_.line_info);
}

BOOST_AUTO_TEST_CASE(info_quiet)
{
    log_.set_verbose(false);
    log_.info("info");

    BOOST_REQUIRE(log_.line_info.empty());
}

BOOST_AUTO_TEST_CASE(warning)
{
    log_.warning("warning");

    BOOST_REQUIRE_EQUAL("gninraw", log_.line_warning);
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
