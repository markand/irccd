/*
 * logger.h -- irccd logging
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

#ifndef IRCCD_LOGGER_H
#define IRCCD_LOGGER_H

#include <irccd/sysconfig.h>

#include <memory>
#include <sstream>
#include <utility>

namespace irccd {

namespace log {

/**
 * @enum Level
 * @brief Which level of warning
 */
enum class Level {
	Info,			//!< Standard information (disabled if verbose is false)
	Warning,		//!< Warning (always shown)
	Debug			//!< Debug message (only if compiled in debug mode)
};

/* --------------------------------------------------------
 * Interface -- abstract logging interface
 * -------------------------------------------------------- */

/**
 * @class Interface
 * @brief Interface to implement new logger mechanisms
 *
 * Derive from this class and use Logger::setInterface() to change logging
 * system.
 *
 * @see File
 * @see Console
 * @see Syslog
 * @see Silent
 */
class Interface {
public:
	/**
	 * Write the line to the logs. The line to write will never contains
	 * trailing new line character.
	 *
	 * @param level the level
	 * @param line the line without trailing \n
	 */	
	virtual void write(Level level, const std::string &line) noexcept = 0;
};

/* --------------------------------------------------------
 * Console -- logs to console
 * -------------------------------------------------------- */

/**
 * @class Console
 * @brief Logger implementation for console output
 */
class Console : public Interface {
public:
	/**
	 * @copydoc Interface::write
	 */
	void write(Level level, const std::string &line) noexcept override;
};

/* --------------------------------------------------------
 * File -- logs to a file
 * -------------------------------------------------------- */

/**
 * @class File
 * @brief Output to a file
 */
class File : public Interface {
private:
	std::string m_outputNormal;
	std::string m_outputError;

public:
	/**
	 * Outputs to files. Info and Debug are written in normal and Warnings
	 * in errors.
	 *
	 * The same path can be used for all levels.
	 *
	 * @param normal the path to the normal logs
	 * @param errors the path to the errors logs
	 */
	File(std::string normal, std::string errors);

	/**
	 * @copydoc Interface::write
	 */
	void write(Level level, const std::string &line) noexcept override;
};

/* --------------------------------------------------------
 * Silent -- disable all logs
 * -------------------------------------------------------- */

/**
 * @class Silent
 * @brief Use to disable logs
 *
 * Useful for unit tests when some classes may emits log.
 */
class Silent : public Interface {
public:
	/**
	 * @copydoc Interface::write
	 */
	void write(Level level, const std::string &line) noexcept override;
};

/* --------------------------------------------------------
 * Syslog -- system logger
 * -------------------------------------------------------- */

#if defined(HAVE_SYSLOG)

/**
 * @class Syslog
 * @brief Implements logger into syslog
 */
class Syslog : public Interface {
public:
	/**
	 * Open the syslog.
	 */
	Syslog();

	/**
	 * Close the syslog.
	 */
	~Syslog();

	/**
	 * @copydoc Interface::write
	 */
	void write(Level level, const std::string &line) noexcept override;
};

#endif // !HAVE_SYSLOG

/* --------------------------------------------------------
 * Functions
 * -------------------------------------------------------- */

/**
 * Update the logger interface.
 *
 * @param iface the new interface
 */
void setInterface(std::unique_ptr<Interface> iface) noexcept;

/**
 * Get the stream for informational messages.
 *
 * @return the stream
 * @note Has no effect if verbose is set to false.
 */
std::ostream &info() noexcept;

/**
 * Get the stream for warnings.
 *
 * @return the stream
 */
std::ostream &warning() noexcept;

/**
 * Get the stream for debug messages.
 *
 * @return the stream
 * @note Has no effect if compiled in release mode.
 */
std::ostream &debug() noexcept;

/**
 * Tells if verbose is enabled.
 *
 * @return true if enabled
 */
bool isVerbose() noexcept;

/**
 * Set the verbosity mode.
 *
 * @param mode the new mode
 */
void setVerbose(bool mode) noexcept;

} // !log

} // !irccd

#endif // !IRCCD_LOGGER_H
