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

#include <memory>
#include <sstream>
#include <utility>

#include "sysconfig.hpp"

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
	 * Default constructor.
	 */
	Interface() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~Interface() = default;

	/**
	 * Write a debug message.
	 *
	 * This function is called only if NDEBUG is not defined.
	 *
	 * \param data the data
	 * \see log::debug
	 */
	virtual void debug(const std::string &line) = 0;

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
};

/*
 * Filter -- modify messages before printing
 * ------------------------------------------------------------------
 */

/**
 * \brief Filter messages before printing them.
 *
 * Derive from this class and use log::setFilter.
 */
class Filter {
public:
	/**
	 * Default constructor.
	 */
	Filter() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~Filter() = default;

	/**
	 * Update the debug message.
	 *
	 * \param input the message
	 * \return the updated message
	 */
	virtual std::string preDebug(std::string input) const
	{
		return input;
	}

	/**
	 * Update the information message.
	 *
	 * \param input the message
	 * \return the updated message
	 */
	virtual std::string preInfo(std::string input) const
	{
		return input;
	}

	/**
	 * Update the warning message.
	 *
	 * \param input the message
	 * \return the updated message
	 */
	virtual std::string preWarning(std::string input) const
	{
		return input;
	}
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
	 * \copydoc Interface::debug
	 */
	IRCCD_EXPORT void debug(const std::string &line) override;

	/**
	 * \copydoc Interface::info
	 */
	IRCCD_EXPORT void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	IRCCD_EXPORT void warning(const std::string &line) override;
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
	std::string m_outputNormal;
	std::string m_outputError;

public:
	/**
	 * Outputs to files.
	 *
	 * \param normal the path to the normal logs
	 * \param errors the path to the errors logs
	 */
	IRCCD_EXPORT File(std::string normal, std::string errors);

	/**
	 * \copydoc Interface::debug
	 */
	IRCCD_EXPORT void debug(const std::string &line) override;

	/**
	 * \copydoc Interface::info
	 */
	IRCCD_EXPORT void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	IRCCD_EXPORT void warning(const std::string &line) override;
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
	 * \copydoc Interface::debug
	 */
	IRCCD_EXPORT void debug(const std::string &line) override;

	/**
	 * \copydoc Interface::info
	 */
	IRCCD_EXPORT void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	IRCCD_EXPORT void warning(const std::string &line) override;
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
	IRCCD_EXPORT Syslog();

	/**
	 * Close the syslog.
	 */
	IRCCD_EXPORT ~Syslog();

	/**
	 * \copydoc Interface::debug
	 */
	IRCCD_EXPORT void debug(const std::string &line) override;

	/**
	 * \copydoc Interface::info
	 */
	IRCCD_EXPORT void info(const std::string &line) override;

	/**
	 * \copydoc Interface::warning
	 */
	IRCCD_EXPORT void warning(const std::string &line) override;
};

#endif // !HAVE_SYSLOG

/*
 * Functions
 * ------------------------------------------------------------------
 */

/**
 * Update the logger interface.
 *
 * \pre iface must not be null
 * \param iface the new interface
 */
IRCCD_EXPORT void setInterface(std::unique_ptr<Interface> iface) noexcept;

/**
 * Set an optional filter.
 *
 * \pre filter must not be null
 * \param filter the filter
 */
IRCCD_EXPORT void setFilter(std::unique_ptr<Filter> filter) noexcept;

/**
 * Get the stream for informational messages.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 * \note Has no effect if verbose is set to false.
 */
IRCCD_EXPORT std::ostream &info(const std::string &message = "");

/**
 * Get the stream for warnings.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 */
IRCCD_EXPORT std::ostream &warning(const std::string &message = "");

/**
 * Get the stream for debug messages.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 * \note Has no effect if compiled in release mode.
 */
IRCCD_EXPORT std::ostream &debug(const std::string &message = "");

/**
 * Tells if verbose is enabled.
 *
 * \return true if enabled
 */
IRCCD_EXPORT bool isVerbose() noexcept;

/**
 * Set the verbosity mode.
 *
 * \param mode the new mode
 */
IRCCD_EXPORT void setVerbose(bool mode) noexcept;

} // !log

} // !irccd

#endif // !IRCCD_LOGGER_HPP
