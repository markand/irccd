/*
 * irccd.hpp -- main irccd class
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

#ifndef IRCCD_HPP
#define IRCCD_HPP

/**
 * \file irccd.hpp
 * \brief Base class for irccd front end.
 */

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include <boost/asio.hpp>

#include "config.hpp"
#include "net.hpp"
#include "sysconfig.hpp"

/**
 * \brief Main irccd namespace
 */
namespace irccd {

class command_service;
class interrupt_service;
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

    // Main loop stuff.
    std::atomic<bool> running_{true};
    std::mutex mutex_;
    std::vector<std::function<void (irccd&)>> events_;

    // Services.
    std::shared_ptr<command_service> command_service_;
    std::shared_ptr<interrupt_service> itr_service_;
    std::shared_ptr<server_service> server_service_;
    std::shared_ptr<transport_service> tpt_service_;
    std::shared_ptr<rule_service> rule_service_;
    std::shared_ptr<plugin_service> plugin_service_;

    // Not copyable and not movable because services has references to irccd.
    irccd(const irccd&) = delete;
    irccd(irccd&&) = delete;

    irccd& operator=(const irccd&) = delete;
    irccd& operator=(irccd&&) = delete;

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
     * Prepare the services for selection.
     *
     * \param in the input set
     * \param out the output set
     * \param max the maximum handle
     */
    void prepare(fd_set& in, fd_set& out, net::Handle& max);

    /**
     * Synchronize the services.
     *
     * \param in the input set
     * \param out the output set
     */
    void sync(fd_set& in, fd_set& out);

    /**
     * Add an event to the queue. This will immediately signals the event loop
     * to interrupt itself to dispatch the pending events.
     *
     * \param ev the event
     * \note Thread-safe
     */
    void post(std::function<void (irccd&)> ev) noexcept;

    /**
     * Loop forever by calling prepare and sync indefinitely.
     */
    void run();

    /**
     * Request to stop, usually from a signal.
     */
    void stop();
};

} // !irccd

#endif // !IRCCD_HPP
