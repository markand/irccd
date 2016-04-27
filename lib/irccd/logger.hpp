/*
 * logger.hpp -- irccd logging
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

#ifndef IRCCD_LOGGER_HPP
#define IRCCD_LOGGER_HPP

/**
 * \file logger.hpp
 * \brief Logging facilities.
 */

#include "sysconfig.hpp"

#include <memory>
#include <sstream>
#include <utility>

namespace irccd {

namespace log {

/*
 * Interface -- abstract logging interface
 * ------------------------------------------------------------------
 */

/**
 * \brief Interface to implement new logger mechanisms.
 *
 * Derive from this class and use log::setInterface() to change logging system.
 *
 * \see File
 * \see Console
 * \see Syslog
 * \see Silent
 */
class Interface {
public:
	/**
	 * Write a information message.
	 *
	 * The function is called only if verbose is true.
	 *
	 * \param data the data
	 * \see log::info
	 */
	virtual void info(const std::string &line) = 0;

	/**
	 * Write an error message.
	 *
	 * This function is always called.
	 *
	 * \param data the data
	 * \see log::warning
	 */
	virtual void warning(const std::string &line) = 0;

	/**
	 * Write a debug message.
	 *
	 * This function is called only if NDEBUG is not defined.
	 *
	 * \param data the data
	 * \see log::debug
	 */
	virtual void debug(const std::string &line) = 0;
};

/*
 * Console -- logs to console
 * ------------------------------------------------------------------
 */

/**
 * \brief Logger implementation for console output using std::cout and std::cerr.
 */
class Console : public Interface {
public:
	/**
	 * \copydoc Interface::info
	 */
	void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	void warning(const std::string &line) override;

	/**
	 * \copydoc Interface::debug
	 */
	void debug(const std::string &line) override;
};

/*
 * File -- logs to a file
 * ------------------------------------------------------------------
 */

/**
 * \brief Output to a files.
 */
class File : public Interface {
private:
	std::string m_output_normal;
	std::string m_output_error;

public:
	/**
	 * Outputs to files.
	 *
	 * \param normal the path to the normal logs
	 * \param errors the path to the errors logs
	 */
	File(std::string normal, std::string errors);

	/**
	 * \copydoc Interface::info
	 */
	void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	void warning(const std::string &line) override;

	/**
	 * \copydoc Interface::debug
	 */
	void debug(const std::string &line) override;
};

/*
 * Silent -- disable all logs
 * ------------------------------------------------------------------
 */

/**
 * \brief Use to disable logs.
 *
 * Useful for unit tests when some classes may emits log.
 */
class Silent : public Interface {
public:
	/**
	 * \copydoc Interface::info
	 */
	void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	void warning(const std::string &line) override;

	/**
	 * \copydoc Interface::debug
	 */
	void debug(const std::string &line) override;
};

/*
 * Syslog -- system logger
 * ------------------------------------------------------------------
 */

#if defined(HAVE_SYSLOG)

/**
 * \brief Implements logger into syslog.
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
	 * \copydoc Interface::info
	 */
	void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	void warning(const std::string &line) override;

	/**
	 * \copydoc Interface::debug
	 */
	void debug(const std::string &line) override;
};

#endif // !HAVE_SYSLOG

/*
 * Functions
 * ------------------------------------------------------------------
 */

/**
 * Update the logger interface.
 *
 * \param iface the new interface
 */
void setInterface(std::unique_ptr<Interface> iface) noexcept;

/**
 * Get the stream for informational messages.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 * \note Has no effect if verbose is set to false.
 */
std::ostream &info(const std::string &message = "");

/**
 * Get the stream for warnings.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 */
std::ostream &warning(const std::string &message = "");

/**
 * Get the stream for debug messages.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 * \note Has no effect if compiled in release mode.
 */
std::ostream &debug(const std::string &message = "");

/**
 * Tells if verbose is enabled.
 *
 * \return true if enabled
 */
bool isVerbose() noexcept;

/**
 * Set the verbosity mode.
 *
 * \param mode the new mode
 */
void setVerbose(bool mode) noexcept;

} // !log

} // !irccd

#endif // !IRCCD_LOGGER_HPP
