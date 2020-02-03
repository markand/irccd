/*
 * logger.cpp -- irccd logging
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

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <streambuf>

#include "logger.hpp"

#if defined(IRCCD_HAVE_SYSLOG)
#	include <syslog.h>
#endif // !IRCCD_HAVE_SYSLOG

namespace irccd::daemon::logger {

void logger::debug(const std::string& line)
{
	// Print only in debug mode, the buffer is flushed anyway.
#if !defined(NDEBUG)
	parent_.write_debug(parent_.filter_->pre_debug(category_, component_, line));
#else
	(void)line;
#endif
}

void logger::info(const std::string& line)
{
	// Print only if verbose, the buffer will be flushed anyway.
	if (parent_.verbose_)
		parent_.write_info(parent_.filter_->pre_info(category_, component_, line));
}

void logger::warning(const std::string& line)
{
	parent_.write_warning(parent_.filter_->pre_warning(category_, component_, line));
}

logger::logger(sink& parent, level level, std::string_view category, std::string_view component) noexcept
	: std::ostream(this)
	, level_(level)
	, parent_(parent)
	, category_(category)
	, component_(component)
{
	assert(level >= level::debug && level <= level::warning);
}

int logger::sync()
{
	std::string buffer = str();
	std::string::size_type pos;

	while ((pos = buffer.find("\n")) != std::string::npos) {
		auto line = buffer.substr(0, pos);

		// Remove this line.
		buffer.erase(buffer.begin(), buffer.begin() + pos + 1);

		switch (level_) {
		case level::debug:
			debug(line);
			break;
		case level::info:
			info(line);
			break;
		case level::warning:
			warning(line);
			break;
		default:
			break;
		}
	}

	str(buffer);

	return 0;
}

void console_sink::write_info(const std::string& line)
{
	std::cout << line << std::endl;
}

void console_sink::write_warning(const std::string& line)
{
	std::cerr << line << std::endl;
}

void console_sink::write_debug(const std::string& line)
{
	std::cout << line << std::endl;
}

file_sink::file_sink(std::string normal, std::string errors)
	: output_normal_(std::move(normal))
	, output_error_(std::move(errors))
{
}

void file_sink::write_info(const std::string& line)
{
	std::ofstream(output_normal_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void file_sink::write_warning(const std::string& line)
{
	std::ofstream(output_error_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void file_sink::write_debug(const std::string& line)
{
	std::ofstream(output_normal_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void silent_sink::write_info(const std::string&)
{
}

void silent_sink::write_warning(const std::string&)
{
}

void silent_sink::write_debug(const std::string&)
{
}

#if defined(IRCCD_HAVE_SYSLOG)

syslog_sink::syslog_sink()
{
	openlog("irccd", LOG_PID, LOG_DAEMON);
}

syslog_sink::~syslog_sink()
{
	closelog();
}

void syslog_sink::write_info(const std::string& line)
{
	syslog(LOG_INFO | LOG_USER, "%s", line.c_str());
}

void syslog_sink::write_warning(const std::string& line)
{
	syslog(LOG_WARNING | LOG_USER, "%s", line.c_str());
}

void syslog_sink::write_debug(const std::string& line)
{
	syslog(LOG_DEBUG | LOG_USER, "%s", line.c_str());
}

#endif // !IRCCD_HAVE_SYSLOG

sink::sink()
	: filter_(new filter)
{
}

auto sink::is_verbose() const noexcept -> bool
{
	return verbose_;
}

void sink::set_verbose(bool mode) noexcept
{
	verbose_ = mode;
}

void sink::set_filter(filter& filter) noexcept
{
	filter_ = &filter;
}

auto sink::info(std::string_view category, std::string_view component) -> logger
{
	return logger(*this, logger::level::info, category, component);;
}

auto sink::warning(std::string_view category, std::string_view component) -> logger
{
	return logger(*this, logger::level::warning, category, component);;
}

auto sink::debug(std::string_view category, std::string_view component) -> logger
{
	return logger(*this, logger::level::debug, category, component);;
}

auto filter::pre(std::string_view category,
                 std::string_view component,
                 std::string_view message) const -> std::string
{
	std::ostringstream oss;

	oss << category;

	if (!component.empty())
		oss << " " << component;

	oss << ": ";
	oss << message;

	return oss.str();
}

auto filter::pre_debug(std::string_view category,
                       std::string_view component,
                       std::string_view message) const -> std::string
{
	return pre(category, component, message);
}

auto filter::pre_info(std::string_view category,
                      std::string_view component,
                      std::string_view message) const -> std::string
{
	return pre(category, component, message);
}

auto filter::pre_warning(std::string_view category,
                         std::string_view component,
                         std::string_view message) const -> std::string
{
	return pre(category, component, message);
}

} // !irccd::daemon::logger
