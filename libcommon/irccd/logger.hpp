/*
 * logger.hpp -- irccd logging
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
 * logger -- abstract logging interface
 * ------------------------------------------------------------------
 */

/**
 * \brief Interface to implement new logger mechanisms.
 *
 * Derive from this class and use log::set_logger() to change logging system.
 *
 * \see file_logger
 * \see console_logger
 * \see syslog_logger
 * \see silent_logger
 */
class logger {
public:
    /**
     * Virtual destructor defaulted.
     */
    virtual ~logger() = default;

    /**
     * Write a debug message.
     *
     * This function is called only if NDEBUG is not defined.
     *
     * \param line the data
     * \see log::debug
     */
    virtual void debug(const std::string& line) = 0;

    /**
     * Write a information message.
     *
     * The function is called only if verbose is true.
     *
     * \param line the data
     * \see log::info
     */
    virtual void info(const std::string& line) = 0;

    /**
     * Write an error message.
     *
     * This function is always called.
     *
     * \param line the data
     * \see log::warning
     */
    virtual void warning(const std::string& line) = 0;
};

/*
 * filter -- modify messages before printing
 * ------------------------------------------------------------------
 */

/**
 * \brief Filter messages before printing them.
 *
 * Derive from this class and use log::setFilter.
 */
class filter {
public:
    /**
     * Virtual destructor defaulted.
     */
    virtual ~filter() = default;

    /**
     * Update the debug message.
     *
     * \param input the message
     * \return the updated message
     */
    virtual std::string pre_debug(std::string input) const
    {
        return input;
    }

    /**
     * Update the information message.
     *
     * \param input the message
     * \return the updated message
     */
    virtual std::string pre_info(std::string input) const
    {
        return input;
    }

    /**
     * Update the warning message.
     *
     * \param input the message
     * \return the updated message
     */
    virtual std::string pre_warning(std::string input) const
    {
        return input;
    }
};

/*
 * console_logger -- logs to console
 * ------------------------------------------------------------------
 */

/**
 * \brief Logger implementation for console output using std::cout and
 *        std::cerr.
 */
class console_logger : public logger {
public:
    /**
     * \copydoc logger::debug
     */
    void debug(const std::string& line) override;

    /**
     * \copydoc logger::info
     */
    void info(const std::string& line) override;

    /**
     * \copydoc logger::warning
     */
    void warning(const std::string& line) override;
};

/*
 * file_logger -- logs to a file
 * ------------------------------------------------------------------
 */

/**
 * \brief Output to a files.
 */
class file_logger : public logger {
private:
    std::string output_normal_;
    std::string output_error_;

public:
    /**
     * Outputs to files.
     *
     * \param normal the path to the normal logs
     * \param errors the path to the errors logs
     */
    file_logger(std::string normal, std::string errors);

    /**
     * \copydoc logger::debug
     */
    void debug(const std::string& line) override;

    /**
     * \copydoc logger::info
     */
    void info(const std::string& line) override;

    /**
     * \copydoc logger::warning
     */
    void warning(const std::string& line) override;
};

/*
 * silent_logger -- disable all logs
 * ------------------------------------------------------------------
 */

/**
 * \brief Use to disable logs.
 *
 * Useful for unit tests when some classes may emits log.
 */
class silent_logger : public logger {
public:
    /**
     * \copydoc logger::debug
     */
    void debug(const std::string& line) override;

    /**
     * \copydoc logger::info
     */
    void info(const std::string& line) override;

    /**
     * \copydoc logger::warning
     */
    void warning(const std::string& line) override;
};

/*
 * syslog_logger -- system logger
 * ------------------------------------------------------------------
 */

#if defined(HAVE_SYSLOG)

/**
 * \brief Implements logger into syslog.
 */
class syslog_logger : public logger {
public:
    /**
     * Open the syslog.
     */
    syslog_logger();

    /**
     * Close the syslog.
     */
    ~syslog_logger();

    /**
     * \copydoc logger::debug
     */
    void debug(const std::string& line) override;

    /**
     * \copydoc logger::info
     */
    void info(const std::string& line) override;

    /**
     * \copydoc logger::warning
     */
    void warning(const std::string& line) override;
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
IRCCD_EXPORT void set_logger(std::unique_ptr<logger> iface) noexcept;

/**
 * Set an optional filter.
 *
 * \pre filter must not be null
 * \param filter the filter
 */
IRCCD_EXPORT void set_filter(std::unique_ptr<filter> filter) noexcept;

/**
 * Get the stream for informational messages.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 * \note Has no effect if verbose is set to false.
 */
IRCCD_EXPORT std::ostream& info(const std::string& message = "");

/**
 * Get the stream for warnings.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 */
IRCCD_EXPORT std::ostream& warning(const std::string& message = "");

/**
 * Get the stream for debug messages.
 *
 * If message is specified, a new line character is appended.
 *
 * \param message the optional message to write
 * \return the stream
 * \note Has no effect if compiled in release mode.
 */
IRCCD_EXPORT std::ostream& debug(const std::string& message = "");

/**
 * Tells if verbose is enabled.
 *
 * \return true if enabled
 */
IRCCD_EXPORT bool is_verbose() noexcept;

/**
 * Set the verbosity mode.
 *
 * \param mode the new mode
 */
IRCCD_EXPORT void set_verbose(bool mode) noexcept;

} // !log

} // !irccd

#endif // !IRCCD_LOGGER_HPP
