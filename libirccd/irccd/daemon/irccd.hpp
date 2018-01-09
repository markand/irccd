/*
 * irccd.hpp -- main irccd class
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

#ifndef IRCCD_DAEMON_IRCCD_HPP
#define IRCCD_DAEMON_IRCCD_HPP

/**
 * \file irccd.hpp
 * \brief Base class for irccd front end.
 */

#include <irccd/sysconfig.hpp>

#include <memory>

#include <boost/asio.hpp>

#include <irccd/config.hpp>

/**
 * \brief Main irccd namespace
 */
namespace irccd {

class command_service;
class logger;
class plugin_service;
class rule_service;
class server_service;
class transport_service;

/**
 * \brief Irccd main instance.
 */
class irccd {
private:
    // Configurations.
    class config config_;

    // Main io service.
    boost::asio::io_service& service_;

    // Tells if the configuration has already been called.
    bool loaded_{false};

    // Custom logger.
    std::unique_ptr<logger> logger_;

    // Services.
    std::shared_ptr<command_service> command_service_;
    std::shared_ptr<server_service> server_service_;
    std::shared_ptr<transport_service> tpt_service_;
    std::shared_ptr<rule_service> rule_service_;
    std::shared_ptr<plugin_service> plugin_service_;

    // Not copyable and not movable because services has references to irccd.
    irccd(const irccd&) = delete;
    irccd(irccd&&) = delete;

    irccd& operator=(const irccd&) = delete;
    irccd& operator=(irccd&&) = delete;

    // Load functions.
    void load_logs_file(const ini::section&);
    void load_logs_syslog();
    void load_logs();
    void load_formats();
    void load_pid();
    void load_gid();
    void load_uid();

public:
    /**
     * Prepare standard services.
     *
     * \param service the service
     * \param config the optional path to the configuration.
     */
    irccd(boost::asio::io_service& service, std::string config = "");

    /**
     * Default destructor.
     */
    ~irccd();

    /**
     * Get the current configuration.
     *
     * \return the configuration
     */
    inline const class config& config() const noexcept
    {
        return config_;
    }

    /**
     * Set the configuration.
     *
     * \param cfg the new config
     */
    inline void set_config(class config cfg) noexcept
    {
        config_ = std::move(cfg);
    }

    /**
     * Get the underlying io service.
     *
     * \return the service
     */
    inline const boost::asio::io_service& service() const noexcept
    {
        return service_;
    }

    /**
     * Overloaded function.
     *
     * \return the service
     */
    inline boost::asio::io_service& service() noexcept
    {
        return service_;
    }

    /**
     * Access the logger.
     *
     * \return the logger
     */
    inline const logger& log() const noexcept
    {
        return *logger_;
    }

    /**
     * Overloaded function.
     *
     * \return the logger
     */
    inline logger& log() noexcept
    {
        return *logger_;
    }

    /**
     * Set the logger.
     *
     * \pre logger != nullptr
     * \param logger the new logger
     */
    void set_log(std::unique_ptr<logger> logger) noexcept;

    /**
     * Access the command service.
     *
     * \return the service
     */
    inline command_service& commands() noexcept
    {
        return *command_service_;
    }

    /**
     * Access the server service.
     *
     * \return the service
     */
    inline server_service& servers() noexcept
    {
        return *server_service_;
    }

    /**
     * Access the transport service.
     *
     * \return the service
     */
    inline transport_service& transports() noexcept
    {
        return *tpt_service_;
    }

    /**
     * Access the rule service.
     *
     * \return the service
     */
    inline rule_service& rules() noexcept
    {
        return *rule_service_;
    }

    /**
     * Access the plugin service.
     *
     * \return the service
     */
    inline plugin_service& plugins() noexcept
    {
        return *plugin_service_;
    }

    /**
     * Load and re-apply the configuration to the daemon.
     */
    void load() noexcept;
};

/**
 * \brief Irccd error.
 */
class irccd_error : public boost::system::system_error {
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
const boost::system::error_category& irccd_category();

/**
 * Create a boost::system::error_code from irccd_error::error enum.
 *
 * \param e the error code
 */
boost::system::error_code make_error_code(irccd_error::error e);

} // !irccd

namespace boost {

namespace system {

template <>
struct is_error_code_enum<irccd::irccd_error::error> : public std::true_type {
};

} // !system

} // !boost

#endif // !IRCCD_DAEMON_IRCCD_HPP
