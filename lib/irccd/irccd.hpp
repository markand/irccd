/*
 * irccd.hpp -- main irccd class
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

#include "net.hpp"
#include "sysconfig.hpp"

/**
 * \brief Main irccd namespace
 */
namespace irccd {

class CommandService;
class InterruptService;
class ModuleService;
class PluginService;
class RuleService;
class ServerService;
class TransportService;

/**
 * \class Irccd
 * \brief Irccd main instance.
 */
class Irccd {
private:
    // Main loop stuff.
    std::atomic<bool> m_running{true};
    std::mutex m_mutex;
    std::vector<std::function<void (Irccd &)>> m_events;

    // Services.
    std::shared_ptr<CommandService> m_commandService;
    std::shared_ptr<InterruptService> m_interruptService;
    std::shared_ptr<ServerService> m_servers;
    std::shared_ptr<TransportService> m_transports;
    std::shared_ptr<RuleService> m_ruleService;
    std::shared_ptr<ModuleService> m_moduleService;
    std::shared_ptr<PluginService> m_plugins;

    // Not copyable and not movable because services has references to irccd.
    Irccd(const Irccd &) = delete;
    Irccd(Irccd &&) = delete;

    Irccd &operator=(const Irccd &) = delete;
    Irccd &operator=(Irccd &&) = delete;

public:
    /**
     * Prepare standard services.
     */
    IRCCD_EXPORT Irccd();

    /**
     * Access the command service.
     *
     * \return the service
     */
    inline CommandService &commands() noexcept
    {
        return *m_commandService;
    }

    /**
     * Access the server service.
     *
     * \return the service
     */
    inline ServerService &servers() noexcept
    {
        return *m_servers;
    }

    /**
     * Access the transport service.
     *
     * \return the service
     */
    inline TransportService &transports() noexcept
    {
        return *m_transports;
    }

    /**
     * Access the rule service.
     *
     * \return the service
     */
    inline RuleService &rules() noexcept
    {
        return *m_ruleService;
    }

    /**
     * Access the module service.
     *
     * \return the service
     */
    inline ModuleService &modules() noexcept
    {
        return *m_moduleService;
    }

    /**
     * Access the plugin service.
     *
     * \return the service
     */
    inline PluginService &plugins() noexcept
    {
        return *m_plugins;
    }

    /**
     * Prepare the services for selection.
     *
     * \param in the input set
     * \param out the output set
     * \param max the maximum handle
     */
    IRCCD_EXPORT void prepare(fd_set &in, fd_set &out, net::Handle &max);

    /**
     * Synchronize the services.
     *
     * \param in the input set
     * \param out the output set
     */
    IRCCD_EXPORT void sync(fd_set &in, fd_set &out);

    /**
     * Add an event to the queue. This will immediately signals the event loop
     * to interrupt itself to dispatch the pending events.
     *
     * \param ev the event
     * \note Thread-safe
     */
    IRCCD_EXPORT void post(std::function<void (Irccd &)> ev) noexcept;

    /**
     * Loop forever by calling poll() and dispatch() indefinitely.
     */
    IRCCD_EXPORT void run();

    /**
     * Dispatch the pending events, usually after calling poll().
     */
    IRCCD_EXPORT void dispatch();

    /**
     * Request to stop, usually from a signal.
     */
    IRCCD_EXPORT void stop();
};

} // !irccd

#endif // !IRCCD_HPP
