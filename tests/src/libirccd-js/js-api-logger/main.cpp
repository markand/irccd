/*
 * main.cpp -- test Irccd.Logger API
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#include <irccd/test/js_fixture.hpp>

using irccd::daemon::logger::sink;

using irccd::js::duk::get_stack;

using irccd::test::js_fixture;

namespace irccd {

namespace {

class logger_fixture : public js_fixture {
protected:
	std::string line_info;
	std::string line_warning;
	std::string line_debug;

	class sample_sink : public sink {
	private:
		logger_fixture& test_;

	public:
		sample_sink(logger_fixture& test) noexcept
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

	logger_fixture()
	{
		bot_.set_log(std::make_unique<sample_sink>(*this));
		bot_.get_log().set_verbose(true);
	}
};

BOOST_FIXTURE_TEST_SUITE(logger_js_api_suite, logger_fixture)

BOOST_AUTO_TEST_CASE(info)
{
	if (duk_peval_string(plugin_->get_context(), "Irccd.Logger.info(\"hello!\");") != 0)
		throw get_stack(plugin_->get_context(), -1);

	BOOST_TEST("plugin test: hello!" == line_info);
}

BOOST_AUTO_TEST_CASE(warning)
{
	if (duk_peval_string(plugin_->get_context(), "Irccd.Logger.warning(\"FAIL!\");") != 0)
		throw get_stack(plugin_->get_context(), -1);

	BOOST_TEST("plugin test: FAIL!" == line_warning);
}

#if !defined(NDEBUG)

BOOST_AUTO_TEST_CASE(debug)
{
	if (duk_peval_string(plugin_->get_context(), "Irccd.Logger.debug(\"starting\");") != 0)
		throw get_stack(plugin_->get_context(), -1);

	BOOST_TEST("plugin test: starting" == line_debug);
}

#endif

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
