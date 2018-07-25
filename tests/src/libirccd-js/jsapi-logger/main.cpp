/*
 * main.cpp -- test Irccd.Logger API
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

#define BOOST_TEST_MODULE "Logger Javascript API"
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/logger.hpp>

#include <irccd/js/logger_jsapi.hpp>
#include <irccd/js/plugin_jsapi.hpp>

#include <irccd/test/js_test.hpp>

namespace irccd {

namespace {

class logger_test : public js_test<logger_jsapi, plugin_jsapi> {
protected:
    std::string line_info;
    std::string line_warning;
    std::string line_debug;

    class sample_sink : public logger::sink {
    private:
        logger_test& test_;

    public:
        inline sample_sink(logger_test& test) noexcept
            : test_(test)
        {
        }

        void write_info(const std::string& line) override
        {
            test_.line_info = line;
        }

        void write_warning(const std::string& line) override
        {
            test_.line_warning = line;
        }

        void write_debug(const std::string& line) override
        {
            test_.line_debug = line;
        }
    };

    logger_test()
    {
        irccd_.set_log(std::make_unique<sample_sink>(*this));
        irccd_.get_log().set_verbose(true);
    }
};

BOOST_FIXTURE_TEST_SUITE(logger_jsapi_suite, logger_test)

BOOST_AUTO_TEST_CASE(info)
{
    if (duk_peval_string(plugin_->get_context(), "Irccd.Logger.info(\"hello!\");") != 0)
        throw dukx_stack(plugin_->get_context(), -1);

    BOOST_TEST("plugin test: hello!" == line_info);
}

BOOST_AUTO_TEST_CASE(warning)
{
    if (duk_peval_string(plugin_->get_context(), "Irccd.Logger.warning(\"FAIL!\");") != 0)
        throw dukx_stack(plugin_->get_context(), -1);

    BOOST_TEST("plugin test: FAIL!" == line_warning);
}

#if !defined(NDEBUG)

BOOST_AUTO_TEST_CASE(debug)
{
    if (duk_peval_string(plugin_->get_context(), "Irccd.Logger.debug(\"starting\");") != 0)
        throw dukx_stack(plugin_->get_context(), -1);

    BOOST_TEST("plugin test: starting" == line_debug);
}

#endif

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
