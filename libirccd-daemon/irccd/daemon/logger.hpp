/*
 * logger.hpp -- irccd logging
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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
#include <string>
#include <string_view>
#include <utility>

namespace irccd::daemon::logger {

class filter;
class sink;

/**
 * \brief Traits for loggable objects.
 * \ingroup daemon-loggers-traits
 *
 * Specialize this structure and add the following static functions to be able
 * to log object with convenience:
 *
 * ## get_category
 *
 * The get_category function should return a single word that describe the
 * message entry category.
 *
 * Synopsis:
 *
 * ```cpp
 * static auto get_category(const T&) noexcept -> std::string_view;
 * ```
 *
 * ## get_component
 *
 * The get_component function should return the identifier or any valid
 * information about the given object that is useful for the user.
 *
 * If no information could be provided, an empty string can be returned.
 *
 * Synopsis:
 *
 * ```cpp
 * static auto get_component(const T&) noexcept -> std::string_view;
 * ```
 */
template <typename T>
struct type_traits;

/**
 * \brief Logger object.
 * \ingroup daemon-loggers
 */
class logger : public std::ostream, public std::stringbuf {
private:
	/**
	 * \brief Make sink friend.
	 */
	friend class sink;

	enum class level {
		debug,
		info,
		warning
	} level_;

	sink& parent_;

	std::string_view category_;
	std::string_view component_;

	void debug(const std::string&);
	void info(const std::string&);
	void warning(const std::string&);
	auto sync() -> int override;
	logger(sink&, level, std::string_view, std::string_view) noexcept;
};

/**
 * \brief Interface to implement new logger mechanisms.
 * \ingroup daemon-loggers-sinks
 *
 * Derive from this class and implement write_info, write_warning and
 * write_debug functions.
 *
 * \see file_sink
 * \see console_sink
 * \see syslog_sink
 * \see silent_sink
 */
class sink {
private:
	/**
	 * \brief Make logger friend.
	 */
	friend class logger;

	// User options.
	bool verbose_{false};
	std::unique_ptr<filter> filter_;

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
	sink();

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~sink() = default;

	/**
	 * Tells if logger is verbose.
	 *
	 * \return true if verbose
	 */
	auto is_verbose() const noexcept -> bool;

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
	void set_filter(std::unique_ptr<filter> filter) noexcept;

	/**
	 * Get the stream for informational messages.
	 *
	 * If message is specified, a new line character is appended.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \return the output stream
	 * \note Has no effect if verbose is set to false.
	 */
	auto info(std::string_view category, std::string_view component) -> logger;

	/**
	 * Convenient function with loggable objects.
	 *
	 * \param loggable the loggable object
	 * \return the output stream
	 * \see type_traits
	 */
	template <typename Loggable>
	auto info(const Loggable& loggable) -> logger
	{
		return info(
		        type_traits<Loggable>::get_category(loggable),
		        type_traits<Loggable>::get_component(loggable)
		);
	}

	/**
	 * Get the stream for warnings.
	 *
	 * If message is specified, a new line character is appended.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \return the output stream
	 */
	auto warning(std::string_view category, std::string_view component) -> logger;

	/**
	 * Convenient function with loggable objects.
	 *
	 * \param loggable the loggable object
	 * \return the output stream
	 * \see type_traits
	 */
	template <typename Loggable>
	auto warning(const Loggable& loggable) -> logger
	{
		return warning(
		        type_traits<Loggable>::get_category(loggable),
		        type_traits<Loggable>::get_component(loggable)
		);
	}

	/**
	 * Get the stream for debug messages.
	 *
	 * If message is specified, a new line character is appended.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \return the output stream
	 * \note Has no effect if compiled in release mode.
	 */
	auto debug(std::string_view category, std::string_view component) -> logger;

	/**
	 * Convenient function with loggable objects.
	 *
	 * \param loggable the loggable object
	 * \return the output stream
	 * \see type_traits
	 */
	template <typename Loggable>
	auto debug(const Loggable& loggable) -> logger
	{
		return debug(
		        type_traits<Loggable>::get_category(loggable),
		        type_traits<Loggable>::get_component(loggable)
		);
	}
};

/**
 * \brief Filter messages before printing them.
 * \ingroup daemon-loggers
 */
class filter {
public:
	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~filter() = default;

	/**
	 * Default function called for each virtual ones.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \param message the message
	 * \return default formatted message
	 */
	auto pre(std::string_view category,
	         std::string_view component,
	         std::string_view message) const -> std::string;


	/**
	 * Update the debug message.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \param message the message
	 * \return the message
	 */
	virtual auto pre_debug(std::string_view category,
	                       std::string_view component,
	                       std::string_view message) const -> std::string;

	/**
	 * Update the information message.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \param message the message
	 * \return the updated message
	 */
	virtual auto pre_info(std::string_view category,
	                      std::string_view component,
	                      std::string_view message) const -> std::string;

	/**
	 * Update the warning message.
	 *
	 * \param category the category subsystem
	 * \param component the optional component
	 * \param message the message
	 * \return the updated message
	 */
	virtual auto pre_warning(std::string_view category,
	                         std::string_view component,
	                         std::string_view message) const -> std::string;
};

/**
 * \brief Logger implementation for console output using std::cout and
 *        std::cerr.
 * \ingroup daemon-loggers-sinks
 */
class console_sink : public sink {
protected:
	/**
	 * \copydoc sink::write_debug
	 */
	void write_debug(const std::string& line) override;

	/**
	 * \copydoc sink::write_info
	 */
	void write_info(const std::string& line) override;

	/**
	 * \copydoc sink::write_warning
	 */
	void write_warning(const std::string& line) override;
};

/**
 * \brief Output to a files.
 * \ingroup daemon-loggers-sinks
 */
class file_sink : public sink {
private:
	std::string output_normal_;
	std::string output_error_;

protected:
	/**
	 * \copydoc sink::write_debug
	 */
	void write_debug(const std::string& line) override;

	/**
	 * \copydoc sink::write_info
	 */
	void write_info(const std::string& line) override;

	/**
	 * \copydoc sink::write_warning
	 */
	void write_warning(const std::string& line) override;

public:
	/**
	 * Outputs to files.
	 *
	 * \param normal the path to the normal logs
	 * \param errors the path to the errors logs
	 */
	file_sink(std::string normal, std::string errors);
};

/**
 * \brief Use to disable logs.
 * \ingroup daemon-loggers-sinks
 *
 * Useful for unit tests when some classes may emits log.
 */
class silent_sink : public sink {
protected:
	/**
	 * \copydoc sink::write_debug
	 */
	void write_debug(const std::string& line) override;

	/**
	 * \copydoc sink::write_info
	 */
	void write_info(const std::string& line) override;

	/**
	 * \copydoc sink::write_warning
	 */
	void write_warning(const std::string& line) override;
};

#if defined(IRCCD_HAVE_SYSLOG)

/**
 * \brief Implements logger into syslog.
 * \ingroup daemon-loggers-sinks
 */
class syslog_sink : public sink {
protected:
	/**
	 * \copydoc sink::write_debug
	 */
	void write_debug(const std::string& line) override;

	/**
	 * \copydoc sink::write_info
	 */
	void write_info(const std::string& line) override;

	/**
	 * \copydoc sink::write_warning
	 */
	void write_warning(const std::string& line) override;

public:
	/**
	 * Open the syslog.
	 */
	syslog_sink();

	/**
	 * Close the syslog.
	 */
	~syslog_sink();
};

#endif // !IRCCD_HAVE_SYSLOG

} // !irccd::daemon::logger

#endif // !IRCCD_DAEMON_LOGGER_HPP
