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

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <streambuf>

#include <irccd/sysconfig.hpp>

#if defined(HAVE_SYSLOG)
#  include <syslog.h>
#endif // !HAVE_SYSLOG

#include "logger.hpp"
#include "system.hpp"

namespace irccd {

namespace log {

/* --------------------------------------------------------
 * Buffer -- output buffer (private)
 * -------------------------------------------------------- */

/**
 * @class LoggerBuffer
 * @brief This class is a internal buffer for the Logger streams
 */
class Buffer : public std::stringbuf {
private:
	Interface *m_interface{nullptr};
	Level m_level;

public:
	/**
	 * Create the buffer with the specified level.
	 *
	 * @param level the level
	 */
	inline Buffer(Level level) noexcept
		: m_level(level)
	{
	}

	/**
	 * Update the underlying interface.
	 *
	 * @param iface is a non-owning pointer to the new interface
	 */
	inline void setInterface(Interface *iface) noexcept
	{
		m_interface = iface;
	}

	/**
	 * Sync the buffer by calling the interface if set.
	 *
	 * This function split the buffer line per line and remove it before
	 * calling the appropriate interface function.
	 */
	virtual int sync() override
	{
#if defined(NDEBUG)
		/*
		 * Debug is disabled, don't call interface->write() but don't
		 * forget to flush the buffer.
		 */
		if (m_level == Level::Debug) {
			str("");

			return 0;
		}
#endif

		/* Verbose is disabled? Don't show and flush the buffer too. */
		if (m_level == Level::Info && !isVerbose()) {
			str("");

			return 0;
		}

		std::string buffer = str();
		std::string::size_type pos;

		while ((pos = buffer.find("\n")) != std::string::npos) {
			std::string line = buffer.substr(0, pos);

			/* Remove this line */
			buffer.erase(buffer.begin(), buffer.begin() + pos + 1);

			if (m_interface)
				m_interface->write(m_level, line);
		}

		str(buffer);

		return 0;
	}
};

/* --------------------------------------------------------
 * Local variables
 * -------------------------------------------------------- */

namespace {

/* Generic interface for all outputs */
std::unique_ptr<Interface> iface;

/* Internal buffers */
Buffer bufferInfo(Level::Info);
Buffer bufferWarning(Level::Warning);
Buffer bufferDebug(Level::Debug);

/* Stream outputs */
std::ostream streamInfo(&bufferInfo);
std::ostream streamWarning(&bufferWarning);
std::ostream streamDebug(&bufferDebug);

/* Options */
bool verbose(false);

} // !namespace

/* --------------------------------------------------------
 * Console
 * -------------------------------------------------------- */

void Console::write(Level level, const std::string &line) noexcept
{
	if (level == Level::Warning)
		std::cerr << line << std::endl;
	else
		std::cout << line << std::endl;
}

/* --------------------------------------------------------
 * File
 * -------------------------------------------------------- */

File::File(std::string normal, std::string errors)
	: m_outputNormal(std::move(normal))
	, m_outputError(std::move(errors))
{
}

void File::write(Level level, const std::string &line) noexcept
{
	std::string &path = (level == Level::Warning) ? m_outputError : m_outputNormal;
	std::ofstream output(path, std::ofstream::out | std::ofstream::app);

	output << line << std::endl;
}

/* --------------------------------------------------------
 * Silent
 * -------------------------------------------------------- */

void Silent::write(Level, const std::string &) noexcept
{
}

/* --------------------------------------------------------
 * Syslog
 * -------------------------------------------------------- */

#if defined(HAVE_SYSLOG)

Syslog::Syslog()
{
	openlog(sys::programName().c_str(), LOG_PID, LOG_DAEMON);
}

Syslog::~Syslog()
{
	closelog();
}

void Syslog::write(Level level, const std::string &line) noexcept
{
	int syslogLevel;

	switch (level) {
	case Level::Warning:
		syslogLevel = LOG_WARNING;
		break;
	case Level::Debug:
		syslogLevel = LOG_DEBUG;
		break;
	case Level::Info:
		/* FALLTHROUGH */
	default:
		syslogLevel = LOG_INFO;
		break;
	}

	syslog(syslogLevel | LOG_USER, "%s", line.c_str());
}

#endif // !HAVE_SYSLOG

/* --------------------------------------------------------
 * Functions
 * -------------------------------------------------------- */

void setInterface(std::unique_ptr<Interface> ifaceValue) noexcept
{
	iface = std::move(ifaceValue);
	bufferInfo.setInterface(iface.get());
	bufferWarning.setInterface(iface.get());
	bufferDebug.setInterface(iface.get());
}

std::ostream &info() noexcept
{
	return streamInfo;
}

std::ostream &warning() noexcept
{
	return streamWarning;
}

std::ostream &debug() noexcept
{
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
