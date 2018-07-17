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
#include <boost/format.hpp>

#include <irccd/daemon/logger.hpp>

using boost::format;
using boost::str;

namespace irccd {

namespace {

class sample_sink : public logger::sink {
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

class sample_filter : public logger::filter {
public:
    auto pre_debug(std::string_view category,
                   std::string_view component,
                   std::string_view message) const -> std::string override
    {
        return str(format("DEBUG %s:%s:%s") % category % component % message);
    }

    auto pre_info(std::string_view category,
                  std::string_view component,
                  std::string_view message) const -> std::string override
    {
        return str(format("INFO %s:%s:%s") % category % component % message);
    }

    auto pre_warning(std::string_view category,
                     std::string_view component,
                     std::string_view message) const -> std::string override
    {
        return str(format("WARN %s:%s:%s") % category % component % message);
    }
};

class logger_test {
public:
    sample_sink log_;

    logger_test()
    {
        log_.set_filter(std::make_unique<sample_filter>());
        log_.set_verbose(true);
    }
};

BOOST_FIXTURE_TEST_SUITE(logger_test_suite, logger_test)

#if !defined(NDEBUG)

BOOST_AUTO_TEST_CASE(debug)
{
    log_.debug("test", "debug") << "success" << std::endl;

    BOOST_TEST(log_.line_debug == "DEBUG test:debug:success");
}

#endif

BOOST_AUTO_TEST_CASE(info)
{
    log_.info("test", "info") << "success" << std::endl;

    BOOST_TEST(log_.line_info == "INFO test:info:success");
}

BOOST_AUTO_TEST_CASE(info_quiet)
{
    log_.set_verbose(false);
    log_.info("test", "info") << "success" << std::endl;

    BOOST_REQUIRE(log_.line_info.empty());
}

BOOST_AUTO_TEST_CASE(warning)
{
    log_.warning("test", "warning") << "success" << std::endl;

    BOOST_TEST(log_.line_warning == "WARN test:warning:success");
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
