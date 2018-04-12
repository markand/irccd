/*
 * logger.cpp -- irccd logging
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

#include <cassert>
#include <fstream>
#include <iostream>
#include <streambuf>

#include "logger.hpp"

#if defined(HAVE_SYSLOG)
#  include <syslog.h>
#endif // !HAVE_SYSLOG

namespace irccd {

void logger::buffer::debug(std::string line)
{
    // Print only in debug mode, the buffer is flushed anyway.
#if !defined(NDEBUG)
    parent_.write_debug(parent_.filter_->pre_debug(std::move(line)));
#else
    (void)line;
#endif
}

void logger::buffer::info(std::string line)
{
    // Print only if verbose, the buffer will be flushed anyway.
    if (parent_.verbose_)
        parent_.write_info(parent_.filter_->pre_info(std::move(line)));
}

void logger::buffer::warning(std::string line)
{
    parent_.write_warning(parent_.filter_->pre_warning(std::move(line)));
}

logger::buffer::buffer(logger& parent, level level) noexcept
    : parent_(parent)
    , level_(level)
{
    assert(level >= level::debug && level <= level::warning);
}

int logger::buffer::sync()
{
    std::string buffer = str();
    std::string::size_type pos;

    while ((pos = buffer.find("\n")) != std::string::npos) {
        auto line = buffer.substr(0, pos);

        // Remove this line.
        buffer.erase(buffer.begin(), buffer.begin() + pos + 1);

        switch (level_) {
        case level::debug:
            debug(std::move(line));
            break;
        case level::info:
            info(std::move(line));
            break;
        case level::warning:
            warning(std::move(line));
            break;
        default:
            break;
        }
    }

    str(buffer);

    return 0;
}

/*
 * console_logger
 * ------------------------------------------------------------------
 */

void console_logger::write_info(const std::string& line)
{
    std::cout << line << std::endl;
}

void console_logger::write_warning(const std::string& line)
{
    std::cerr << line << std::endl;
}

void console_logger::write_debug(const std::string& line)
{
    std::cout << line << std::endl;
}

/*
 * file_logger
 * ------------------------------------------------------------------
 */

file_logger::file_logger(std::string normal, std::string errors)
    : output_normal_(std::move(normal))
    , output_error_(std::move(errors))
{
}

void file_logger::write_info(const std::string& line)
{
    std::ofstream(output_normal_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void file_logger::write_warning(const std::string& line)
{
    std::ofstream(output_error_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void file_logger::write_debug(const std::string& line)
{
    std::ofstream(output_normal_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

/*
 * silent_logger
 * ------------------------------------------------------------------
 */

void silent_logger::write_info(const std::string&)
{
}

void silent_logger::write_warning(const std::string&)
{
}

void silent_logger::write_debug(const std::string&)
{
}

/*
 * syslog_logger
 * ------------------------------------------------------------------
 */

#if defined(HAVE_SYSLOG)

syslog_logger::syslog_logger()
{
    openlog("irccd", LOG_PID, LOG_DAEMON);
}

syslog_logger::~syslog_logger()
{
    closelog();
}

void syslog_logger::write_info(const std::string& line)
{
    syslog(LOG_INFO | LOG_USER, "%s", line.c_str());
}

void syslog_logger::write_warning(const std::string& line)
{
    syslog(LOG_WARNING | LOG_USER, "%s", line.c_str());
}

void syslog_logger::write_debug(const std::string& line)
{
    syslog(LOG_DEBUG | LOG_USER, "%s", line.c_str());
}

#endif // !HAVE_SYSLOG

/*
 * logger
 * ------------------------------------------------------------------
 */

logger::logger()
    : buffer_info_(*this, buffer::level::info)
    , buffer_warning_(*this, buffer::level::warning)
    , buffer_debug_(*this, buffer::level::debug)
    , stream_info_(&buffer_info_)
    , stream_warning_(&buffer_warning_)
    , stream_debug_(&buffer_debug_)
    , filter_(std::make_unique<logger_filter>())
{
}

bool logger::is_verbose() const noexcept
{
    return verbose_;
}

void logger::set_verbose(bool mode) noexcept
{
    verbose_ = mode;
}

void logger::set_filter(std::unique_ptr<logger_filter> newfilter) noexcept
{
    assert(newfilter);

    filter_ = std::move(newfilter);
}

std::ostream& logger::info(const std::string& message)
{
    if (!message.empty())
        stream_info_ << message << std::endl;

    return stream_info_;
}

std::ostream& logger::warning(const std::string& message)
{
    if (!message.empty())
        stream_warning_ << message << std::endl;

    return stream_warning_;
}

std::ostream& logger::debug(const std::string& message)
{
    if (!message.empty())
        stream_debug_ << message << std::endl;

    return stream_debug_;
}

} // !irccd
