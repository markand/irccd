/*
 * config.hpp -- irccd configuration loader
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

#ifndef IRCCD_CONFIG_HPP
#define IRCCD_CONFIG_HPP

/**
 * \file config.hpp
 * \brief Read .ini configuration file for irccd
 */

#include <memory>
#include <string>
#include <vector>

#include "ini.hpp"
#include "plugin.hpp"
#include "sysconfig.hpp"

namespace irccd {

class irccd;
class rule;
class server;
class transport_server;

/**
 * \brief Read .ini configuration file for irccd
 */
class config {
private:
    std::string path_;
    ini::document document_;

public:
    /**
     * Search the configuration file into the standard defined paths.
     *
     * \return the config
     * \throw std::exception on errors or if no config could be found
     */
    static config find();

    /**
     * Load the configuration from the specified path.
     *
     * \param path the path
     */
    inline config(std::string path = "")
        : path_(std::move(path))
    {
        if (!path_.empty())
            document_ = ini::read_file(path_);
    }

    /**
     * Get the underlying document.
     *
     * \return the document
     */
    inline const ini::document& doc() const noexcept
    {
        return document_;
    }

    /**
     * Get the path to the configuration file.
     *
     * \return the path
     */
    inline const std::string& path() const noexcept
    {
        return path_;
    }

   /**
     * Find an entity if defined in the configuration file.
     *
     * \pre util::isValidIdentifier(name)
     * \param server the server to update
     * \param name the identity name
     * \return default identity if cannot be found
     */
    void load_server_identity(server& server, const std::string& name) const;

    /**
     * Get the path to the pidfile.
     *
     * \return the path or empty if not defined
     */
    std::string pidfile() const;

    /**
     * Get the uid.
     *
     * \return the uid or empty one if no one is set
     */
    std::string uid() const;

    /**
     * Get the gid.
     *
     * \return the gid or empty one if no one is set
     */
    std::string gid() const;

    /**
     * Check if verbosity is enabled.
     *
     * \return true if verbosity was requested
     */
    bool is_verbose() const noexcept;

    /**
     * Check if foreground is specified (= no daemonize).
     *
     * \return true if foreground was requested
     */
    bool is_foreground() const noexcept;

    /**
     * Load logging interface.
     */
    void load_logs() const;

    /**
     * Load formats for logging.
     */
    void load_formats() const;

    /**
     * Load transports.
     *
     * \param irccd the irccd instance
     * \return the set of transports
     */
    std::vector<std::shared_ptr<transport_server>> load_transports(irccd& irccd) const;

    /**
     * Load rules.
     *
     * \return the rules
     */
    std::vector<rule> load_rules() const;

    /**
     * Get the list of servers defined.
     *
     * \return the list of servers
     */
    std::vector<std::shared_ptr<server>> load_servers() const;

    /**
     * Get the list of defined plugins.
     *
     * \param irccd the irccd instance
     * \return the list of plugins
     */
    void load_plugins(irccd& irccd) const;
};

} // !irccd

#endif // !IRCCD_CONFIG_HPP
