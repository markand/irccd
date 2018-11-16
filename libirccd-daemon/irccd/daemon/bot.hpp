/*
 * bot.hpp -- main bot class
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

#ifndef IRCCD_DAEMON_BOT_HPP
#define IRCCD_DAEMON_BOT_HPP

/**
 * \file bot.hpp
 * \brief Base class for irccd front end.
 */

#include <irccd/sysconfig.hpp>

#include <memory>
#include <system_error>

#include <boost/asio/io_service.hpp>

#include <irccd/config.hpp>

/**
 * \brief Main irccd namespace
 */
namespace irccd::daemon {

namespace logger {

class sink;

} // !logger

class plugin_service;
class rule_service;
class server_service;
class transport_service;

/**
 * \brief Irccd main instance.
 */
class bot {
private:
	// Configuration.
	config config_;

	// Main io service.
	boost::asio::io_service& service_;

	// Tells if the configuration has already been called.
	bool loaded_{false};

	// Custom logger.
	std::unique_ptr<logger::sink> sink_;

	// Services.
	std::unique_ptr<server_service> server_service_;
	std::unique_ptr<transport_service> tpt_service_;
	std::unique_ptr<rule_service> rule_service_;
	std::unique_ptr<plugin_service> plugin_service_;

	// Not copyable and not movable because services have references.
	bot(const bot&) = delete;
	bot(bot&&) = delete;

	void operator=(const bot&) = delete;
	void operator=(bot&&) = delete;

	// Load functions.
	void load_logs_file(const ini::section&);
	void load_logs_syslog();
	void load_logs();
	void load_formats();

public:
	/**
	 * Constructor.
	 *
	 * This only create a barebone irccd instance.
	 *
	 * \param service the service
	 * \param config the optional path to the configuration.
	 * \see load_all
	 * \see load_config
	 */
	bot(boost::asio::io_service& service, std::string config = "");

	/**
	 * Default destructor.
	 */
	~bot();

	/**
	 * Get the current configuration.
	 *
	 * \return the configuration
	 */
	auto get_config() const noexcept -> const config&;

	/**
	 * Set the configuration.
	 *
	 * \param cfg the new config
	 */
	void set_config(config cfg) noexcept;

	/**
	 * Get the underlying io service.
	 *
	 * \return the service
	 */
	auto get_service() const noexcept -> const boost::asio::io_service&;

	/**
	 * Overloaded function.
	 *
	 * \return the service
	 */
	auto get_service() noexcept -> boost::asio::io_service&;

	/**
	 * Access the logger.
	 *
	 * \return the logger
	 */
	auto get_log() const noexcept -> const logger::sink&;

	/**
	 * Overloaded function.
	 *
	 * \return the logger
	 */
	auto get_log() noexcept -> logger::sink&;

	/**
	 * Set the logger.
	 *
	 * \pre sink != nullptr
	 * \param sink the new sink
	 */
	void set_log(std::unique_ptr<logger::sink> sink) noexcept;

	/**
	 * Access the server service.
	 *
	 * \return the service
	 */
	auto servers() noexcept -> server_service&;

	/**
	 * Access the transport service.
	 *
	 * \return the service
	 */
	auto transports() noexcept -> transport_service&;

	/**
	 * Access the rule service.
	 *
	 * \return the service
	 */
	auto rules() noexcept -> rule_service&;

	/**
	 * Access the plugin service.
	 *
	 * \return the service
	 */
	auto plugins() noexcept -> plugin_service&;

	/**
	 * Load and re-apply the configuration to the daemon.
	 */
	void load() noexcept;
};

/**
 * \brief Irccd error.
 */
class bot_error : public std::system_error {
public:
	/**
	 * \brief Irccd related errors.
	 */
	enum error {
		//!< No error.
		no_error = 0,

		//!< The connected peer is not irccd.
		not_irccd,

		//!< The irccd version is too different.
		incompatible_version,

		//!< Authentication was required but not issued.
		auth_required,

		//!< Authentication was invalid.
		invalid_auth,

		//!< The message was not a valid JSON object.
		invalid_message,

		//!< The specified command does not exist.
		invalid_command,

		//!< The specified command requires more arguments.
		incomplete_message,
	};

	/**
	 * Inherited constructors.
	 */
	using system_error::system_error;
};

/**
 * Get the irccd error category singleton.
 *
 * \return the singleton
 */
auto bot_category() noexcept -> const std::error_category&;

/**
 * Create a std::error_code from bot_error::error enum.
 *
 * \param e the error code
 * \return the error code
 */
auto make_error_code(bot_error::error e) noexcept -> std::error_code;

} // !irccd::daemon

namespace std {

template <>
struct is_error_code_enum<irccd::daemon::bot_error::error> : public std::true_type {
};

} // !std

#endif // !IRCCD_DAEMON_IRCCD_HPP
