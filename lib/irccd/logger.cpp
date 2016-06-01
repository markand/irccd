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
std::unique_ptr<Filter> filter{new Filter};

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

	void debug(std::string line)
	{
		// Print only in debug mode, the buffer is flushed anyway.
#if !defined(NDEBUG)
		iface->debug(filter->preDebug(std::move(line)));
#else
		(void)line;
#endif
	}

	void info(std::string line)
	{
		// Print only if verbose, the buffer will be flushed anyway.
		if (verbose)
			iface->info(filter->preInfo(std::move(line)));
	}

	void warning(std::string line)
	{
		iface->warning(filter->preWarning(std::move(line)));
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

			// Remove this line.
			buffer.erase(buffer.begin(), buffer.begin() + pos + 1);

			switch (m_level) {
			case Level::Debug:
				debug(std::move(line));
				break;
			case Level::Info:
				info(std::move(line));
				break;
			case Level::Warning:
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
Buffer bufferInfo{Buffer::Info};
Buffer bufferWarning{Buffer::Warning};
Buffer bufferDebug{Buffer::Debug};

// Stream outputs.
std::ostream streamInfo(&bufferInfo);
std::ostream streamWarning(&bufferWarning);
std::ostream streamDebug(&bufferDebug);

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
	: m_outputNormal(std::move(normal))
	, m_outputError(std::move(errors))
{
}

void File::info(const std::string &line)
{
	std::ofstream(m_outputNormal, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void File::warning(const std::string &line)
{
	std::ofstream(m_outputError, std::ofstream::out | std::ofstream::app) << line << std::endl;
}

void File::debug(const std::string &line)
{
	std::ofstream(m_outputNormal, std::ofstream::out | std::ofstream::app) << line << std::endl;
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

void setFilter(std::unique_ptr<Filter> newfilter) noexcept
{
	assert(filter);

	filter = std::move(newfilter);
}

std::ostream &info(const std::string &message)
{
	if (!message.empty())
		streamInfo << message << std::endl;

	return streamInfo;
}

std::ostream &warning(const std::string &message)
{
	if (!message.empty())
		streamWarning << message << std::endl;

	return streamWarning;
}

std::ostream &debug(const std::string &message)
{
	if (!message.empty())
		streamDebug << message << std::endl;

	return streamDebug;
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
