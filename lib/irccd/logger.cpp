/*
 * logger.cpp -- irccd logging
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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
std::unique_ptr<Interface> iface{new Console};

/*
 * Buffer -- output buffer.
 * ------------------------------------------------------------------
 *
 * This class inherits from std::stringbuf and writes the messages to the specified interface function which is one of
 * info, warning and debug.
 */

class Buffer : public std::stringbuf {
public:
	enum Level {
		Debug,
		Info,
		Warning
	};

private:
	Level m_level;

	void debug(const std::string &line)
	{
	/* Print only in debug mode, the buffer is flushed anyway */
#if !defined(NDEBUG)
		iface->debug(line);
#else
		(void)line;
#endif
	}

	void info(const std::string &line)
	{
		/* Print only if verbose, the buffer will be flushed anyway. */
		if (verbose) {
			iface->info(line);
		}
	}

	void warning(const std::string &line)
	{
		iface->warning(line);
	}

public:
	inline Buffer(Level level) noexcept
		: m_level(level)
	{
		assert(level >= Debug && level <= Warning);
	}

	virtual int sync() override
	{
		std::string buffer = str();
		std::string::size_type pos;

		while ((pos = buffer.find("\n")) != std::string::npos) {
			std::string line = buffer.substr(0, pos);

			/* Remove this line */
			buffer.erase(buffer.begin(), buffer.begin() + pos + 1);

			switch (m_level) {
			case Level::Debug:
				debug(line);
				break;
			case Level::Info:
				info(line);
				break;
			case Level::Warning:
				warning(line);
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

/* Information buffer */
Buffer buffer_info{Buffer::Info};

/* Warning buffer */
Buffer buffer_warning{Buffer::Warning};

/* Debug buffer */
Buffer buffer_debug{Buffer::Debug};

/* Stream outputs. */
std::ostream stream_info(&buffer_info);
std::ostream stream_warning(&buffer_warning);
std::ostream stream_debug(&buffer_debug);

} // !namespace

/*
 * Console
 * ------------------------------------------------------------------
 */

void Console::info(const std::string &line)
{
	std::cout << line << std::endl;
}

void Console::warning(const std::string &line)
{
	std::cerr << line << std::endl;
}

void Console::debug(const std::string &line)
{
	std::cout << line << std::endl;
}

/*
 * File
 * ------------------------------------------------------------------
 */

File::File(std::string normal, std::string errors)
	: m_output_normal(std::move(normal))
	, m_output_error(std::move(errors))
{
}

void File::info(const std::string &line)
{
	std::ofstream(m_output_normal, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void File::warning(const std::string &line)
{
	std::ofstream(m_output_error, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void File::debug(const std::string &line)
{
	std::ofstream(m_output_normal, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

/*
 * Silent
 * ------------------------------------------------------------------
 */

void Silent::info(const std::string &)
{
}

void Silent::warning(const std::string &)
{
}

void Silent::debug(const std::string &)
{
}

/*
 * Syslog
 * ------------------------------------------------------------------
 */

#if defined(HAVE_SYSLOG)

Syslog::Syslog()
{
	openlog(sys::programName().c_str(), LOG_PID, LOG_DAEMON);
}

Syslog::~Syslog()
{
	closelog();
}

void Syslog::info(const std::string &line)
{
	syslog(LOG_INFO | LOG_USER, "%s", line.c_str());
}

void Syslog::warning(const std::string &line)
{
	syslog(LOG_WARNING | LOG_USER, "%s", line.c_str());
}

void Syslog::debug(const std::string &line)
{
	syslog(LOG_DEBUG | LOG_USER, "%s", line.c_str());
}

#endif // !HAVE_SYSLOG

/*
 * Functions
 * ------------------------------------------------------------------
 */

void setInterface(std::unique_ptr<Interface> newiface) noexcept
{
	assert(newiface);

	iface = std::move(newiface);
}

std::ostream &info(const std::string &message)
{
	if (!message.empty()) {
		stream_info << message << std::endl;
	}

	return stream_info;
}

std::ostream &warning(const std::string &message)
{
	if (!message.empty()) {
		stream_warning << message << std::endl;
	}

	return stream_warning;
}

std::ostream &debug(const std::string &message)
{
	if (!message.empty()) {
		stream_debug << message << std::endl;
	}

	return stream_debug;
}

bool isVerbose() noexcept
{
	return verbose;
}

void setVerbose(bool mode) noexcept
{
	verbose = mode;
}

} // !log

} // !irccd
