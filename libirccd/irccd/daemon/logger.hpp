/*
 * logger.hpp -- irccd logging
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

#ifndef IRCCD_DAEMON_LOGGER_HPP
#define IRCCD_DAEMON_LOGGER_HPP

/**
 * \file logger.hpp
 * \brief Logging facilities.
 */

#include <irccd/sysconfig.hpp>

#include <memory>
#include <sstream>
#include <utility>

namespace irccd {

class logger_filter;

/**
 * \brief Interface to implement new logger mechanisms.
 *
 * Derive from this class and implement write_info, write_warning and
 * write_debug functions.
 *
 * \see file_logger
 * \see console_logger
 * \see syslog_logger
 * \see silent_logger
 */
class logger {
private:
    class buffer : public std::stringbuf {
    public:
        enum class level {
            debug,
            info,
            warning
        };

    private:
        logger& parent_;
        level level_;

        void debug(std::string line);
        void info(std::string line);
        void warning(std::string line);

    public:
        buffer(logger& parent, level level) noexcept;

        virtual int sync() override;
    };

    // Buffers.
    buffer buffer_info_;
    buffer buffer_warning_;
    buffer buffer_debug_;

    // Stream outputs.
    std::ostream stream_info_;
    std::ostream stream_warning_;
    std::ostream stream_debug_;

    // User options.
    bool verbose_{false};
    std::unique_ptr<logger_filter> filter_;

protected:
    /**
     * Write a debug message.
     *
     * This function is called only if NDEBUG is not defined.
     *
     * \param line the data
     * \see log::debug
     */
    virtual void write_debug(const std::string& line) = 0;

    /**
     * Write a information message.
     *
     * The function is called only if verbose is true.
     *
     * \param line the data
     * \see log::info
     */
    virtual void write_info(const std::string& line) = 0;

    /**
     * Write an error message.
     *
     * This function is always called.
     *
     * \param line the data
     * \see log::warning
     */
    virtual void write_warning(const std::string& line) = 0;

public:
    /**
     * Default constructor.
     */
    logger();

    /**
     * Virtual destructor defaulted.
     */
    virtual ~logger() = default;

    /**
     * Tells if logger is verbose.
     *
     * \return true if verbose
     */
    bool is_verbose() const noexcept;

    /**
     * Set the verbosity mode.
     *
     * \param mode the new mode
     */
    void set_verbose(bool mode) noexcept;

    /**
     * Set an optional filter.
     *
     * \pre filter must not be null
     * \param filter the filter
     */
    void set_filter(std::unique_ptr<logger_filter> filter) noexcept;

    /**
     * Get the stream for informational messages.
     *
     * If message is specified, a new line character is appended.
     *
     * \param message the optional message to write
     * \return the stream
     * \note Has no effect if verbose is set to false.
     */
    std::ostream& info(const std::string& message = "");

    /**
     * Get the stream for warnings.
     *
     * If message is specified, a new line character is appended.
     *
     * \param message the optional message to write
     * \return the stream
     */
    std::ostream& warning(const std::string& message = "");

    /**
     * Get the stream for debug messages.
     *
     * If message is specified, a new line character is appended.
     *
     * \param message the optional message to write
     * \return the stream
     * \note Has no effect if compiled in release mode.
     */
    std::ostream& debug(const std::string& message = "");
};

/**
 * \brief Filter messages before printing them.
 *
 * Derive from this class and use log::setFilter.
 */
class logger_filter {
public:
    /**
     * Virtual destructor defaulted.
     */
    virtual ~logger_filter() = default;

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

/**
 * \brief Logger implementation for console output using std::cout and
 *        std::cerr.
 */
class console_logger : public logger {
protected:
    /**
     * \copydoc logger::write_debug
     */
    void write_debug(const std::string& line) override;

    /**
     * \copydoc logger::write_info
     */
    void write_info(const std::string& line) override;

    /**
     * \copydoc logger::write_warning
     */
    void write_warning(const std::string& line) override;
};

/**
 * \brief Output to a files.
 */
class file_logger : public logger {
private:
    std::string output_normal_;
    std::string output_error_;

protected:
    /**
     * \copydoc logger::write_debug
     */
    void write_debug(const std::string& line) override;

    /**
     * \copydoc logger::write_info
     */
    void write_info(const std::string& line) override;

    /**
     * \copydoc logger::write_warning
     */
    void write_warning(const std::string& line) override;

public:
    /**
     * Outputs to files.
     *
     * \param normal the path to the normal logs
     * \param errors the path to the errors logs
     */
    file_logger(std::string normal, std::string errors);
};

/**
 * \brief Use to disable logs.
 *
 * Useful for unit tests when some classes may emits log.
 */
class silent_logger : public logger {
protected:
    /**
     * \copydoc logger::write_debug
     */
    void write_debug(const std::string& line) override;

    /**
     * \copydoc logger::write_info
     */
    void write_info(const std::string& line) override;

    /**
     * \copydoc logger::write_warning
     */
    void write_warning(const std::string& line) override;
};

#if defined(HAVE_SYSLOG)

/**
 * \brief Implements logger into syslog.
 */
class syslog_logger : public logger {
protected:
    /**
     * \copydoc logger::write_debug
     */
    void write_debug(const std::string& line) override;

    /**
     * \copydoc logger::write_info
     */
    void write_info(const std::string& line) override;

    /**
     * \copydoc logger::write_warning
     */
    void write_warning(const std::string& line) override;

public:
    /**
     * Open the syslog.
     */
    syslog_logger();

    /**
     * Close the syslog.
     */
    ~syslog_logger();
};

#endif // !HAVE_SYSLOG

} // !irccd

#endif // !IRCCD_DAEMON_LOGGER_HPP
