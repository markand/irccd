/*
 * logger.cpp -- irccd logging
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

#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <streambuf>

#include "logger.hpp"
#include "system.hpp"

#if defined(HAVE_SYSLOG)
#  include <syslog.h>
#endif // !HAVE_SYSLOG

namespace irccd {

namespace log {

namespace {

/*
 * User definable options.
 * ------------------------------------------------------------------
 */

std::atomic<bool> verbose{false};
std::unique_ptr<logger> use_iface{new console_logger};
std::unique_ptr<filter> use_filter{new filter};

/*
 * buffer -- output buffer.
 * ------------------------------------------------------------------
 *
 * This class inherits from std::stringbuf and writes the messages to the
 * specified interface function which is one of info, warning and debug.
 */

class buffer : public std::stringbuf {
public:
    enum class level {
        debug,
        info,
        warning
    };

private:
    level level_;

    void debug(std::string line)
    {
        // Print only in debug mode, the buffer is flushed anyway.
#if !defined(NDEBUG)
        use_iface->debug(use_filter->pre_debug(std::move(line)));
#else
        (void)line;
#endif
    }

    void info(std::string line)
    {
        // Print only if verbose, the buffer will be flushed anyway.
        if (verbose)
            use_iface->info(use_filter->pre_info(std::move(line)));
    }

    void warning(std::string line)
    {
        use_iface->warning(use_filter->pre_warning(std::move(line)));
    }

public:
    inline buffer(level level) noexcept
        : level_(level)
    {
        assert(level >= level::debug && level <= level::warning);
    }

    virtual int sync() override
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
};

/*
 * Local variables.
 * ------------------------------------------------------------------
 */

// Buffers.
buffer buffer_info{buffer::level::info};
buffer buffer_warning{buffer::level::warning};
buffer buffer_debug{buffer::level::debug};

// Stream outputs.
std::ostream stream_info(&buffer_info);
std::ostream stream_warning(&buffer_warning);
std::ostream stream_debug(&buffer_debug);

} // !namespace

/*
 * console_logger
 * ------------------------------------------------------------------
 */

void console_logger::info(const std::string& line)
{
    std::cout << line << std::endl;
}

void console_logger::warning(const std::string& line)
{
    std::cerr << line << std::endl;
}

void console_logger::debug(const std::string& line)
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

void file_logger::info(const std::string& line)
{
    std::ofstream(output_normal_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void file_logger::warning(const std::string& line)
{
    std::ofstream(output_error_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void file_logger::debug(const std::string& line)
{
    std::ofstream(output_normal_, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

/*
 * silent_logger
 * ------------------------------------------------------------------
 */

void silent_logger::info(const std::string&)
{
}

void silent_logger::warning(const std::string&)
{
}

void silent_logger::debug(const std::string&)
{
}

/*
 * syslog_logger
 * ------------------------------------------------------------------
 */

#if defined(HAVE_SYSLOG)

syslog_logger::syslog_logger()
{
    openlog(sys::program_name().c_str(), LOG_PID, LOG_DAEMON);
}

syslog_logger::~syslog_logger()
{
    closelog();
}

void syslog_logger::info(const std::string& line)
{
    syslog(LOG_INFO | LOG_USER, "%s", line.c_str());
}

void syslog_logger::warning(const std::string& line)
{
    syslog(LOG_WARNING | LOG_USER, "%s", line.c_str());
}

void syslog_logger::debug(const std::string& line)
{
    syslog(LOG_DEBUG | LOG_USER, "%s", line.c_str());
}

#endif // !HAVE_SYSLOG

/*
 * Functions
 * ------------------------------------------------------------------
 */

void set_logger(std::unique_ptr<logger> new_iface) noexcept
{
    assert(new_iface);

    use_iface = std::move(new_iface);
}

void set_filter(std::unique_ptr<filter> newfilter) noexcept
{
    assert(newfilter);

    use_filter = std::move(newfilter);
}

std::ostream& info(const std::string& message)
{
    if (!message.empty())
        stream_info << message << std::endl;

    return stream_info;
}

std::ostream& warning(const std::string& message)
{
    if (!message.empty())
        stream_warning << message << std::endl;

    return stream_warning;
}

std::ostream& debug(const std::string& message)
{
    if (!message.empty())
        stream_debug << message << std::endl;

    return stream_debug;
}

bool is_verbose() noexcept
{
    return verbose;
}

void set_verbose(bool mode) noexcept
{
    verbose = mode;
}

} // !log

} // !irccd
