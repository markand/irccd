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

#define BOOST_TEST_MODULE "Logger"
#include <boost/test/unit_test.hpp>

#include <irccd/logger.hpp>

namespace irccd {

class logger_test {
public:
    std::string line_debug;
    std::string line_info;
    std::string line_warning;

    class my_logger : public log::logger {
    private:
        logger_test& test_;

    public:
        inline my_logger(logger_test& test) noexcept
            : test_(test)
        {
        }

        void debug(const std::string& line) override
        {
            test_.line_debug = line;
        }

        void info(const std::string& line) override
        {
            test_.line_info = line;
        }

        void warning(const std::string& line) override
        {
            test_.line_warning = line;
        }
    };

    class my_filter : public log::filter {
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

    logger_test()
    {
        log::set_logger(std::make_unique<my_logger>(*this));
        log::set_filter(std::make_unique<my_filter>());
        log::set_verbose(true);
    }
};

BOOST_FIXTURE_TEST_SUITE(logger_test_suite, logger_test)

#if !defined(NDEBUG)

BOOST_AUTO_TEST_CASE(debug)
{
    log::debug("debug");

    BOOST_REQUIRE_EQUAL("gubed", line_debug);
}

#endif

BOOST_AUTO_TEST_CASE(info)
{
    log::info("info");

    BOOST_REQUIRE_EQUAL("ofni", line_info);
}

BOOST_AUTO_TEST_CASE(info_quiet)
{
    log::set_verbose(false);
    log::info("info");

    BOOST_REQUIRE(line_info.empty());
}

BOOST_AUTO_TEST_CASE(warning)
{
    log::warning("warning");

    BOOST_REQUIRE_EQUAL("gninraw", line_warning);
}

BOOST_AUTO_TEST_SUITE_END()

} // !irccd
