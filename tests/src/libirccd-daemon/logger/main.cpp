/*
 * main.cpp -- test logger functions
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

#include <algorithm>
#include <fstream>

#define BOOST_TEST_MODULE "Logger"
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include <irccd/daemon/bot.hpp>
#include <irccd/daemon/logger.hpp>

using boost::format;
using boost::str;

using irccd::daemon::logger::sink;
using irccd::daemon::logger::filter;

namespace irccd {

namespace {

struct sample_sink : public sink {
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

struct sample_filter : public filter {
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
	sample_filter filter_;

	logger_test()
	{
		log_.set_filter(filter_);
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

BOOST_AUTO_TEST_SUITE(logger_config_test_suite)

BOOST_AUTO_TEST_CASE(files)
{
	boost::asio::io_context context;
	daemon::bot bot{context, CMAKE_CURRENT_BINARY_DIR "/logs-files.conf"};

	unlink(CMAKE_CURRENT_BINARY_DIR "/normal.txt");
	unlink(CMAKE_CURRENT_BINARY_DIR "/errors.txt");

	bot.load();
	bot.get_log().info("INFO", "123") << "this is an info" << std::endl;
	bot.get_log().warning("WARNING", "456") << "this is a warning" << std::endl;

	{
		std::ifstream info(CMAKE_CURRENT_BINARY_DIR "/normal.txt");
		std::string line;

		// First line is too early to detect templates.
		std::getline(info, line);
		std::getline(info, line);
		BOOST_TEST(line == "info: INFO=this is an info");
	}

	{
		std::ifstream info(CMAKE_CURRENT_BINARY_DIR "/errors.txt");
		std::string line;

		std::getline(info, line);
		BOOST_TEST(line == "warning: WARNING=this is a warning");
	}
}

BOOST_AUTO_TEST_SUITE_END()

} // !namespace

} // !irccd
