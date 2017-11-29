/*
 * plugin.hpp -- irccd JavaScript plugin interface
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

#ifndef IRCCD_PLUGIN_HPP
#define IRCCD_PLUGIN_HPP

/**
 * \file plugin.hpp
 * \brief irccd plugins
 */

/**
 * \defgroup plugins Plugins
 * \brief Plugin management.
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "server.hpp"
#include "sysconfig.hpp"
#include "util.hpp"

namespace irccd {

class irccd;

/**
 * \brief Configuration map extract from config file.
 */
using plugin_config = std::unordered_map<std::string, std::string>;

/**
 * \brief Formats for plugins.
 */
using plugin_formats = std::unordered_map<std::string, std::string>;

/**
 * \brief Paths for plugins.
 */
using plugin_paths = std::unordered_map<std::string, std::string>;

/**
 * \ingroup plugins
 * \brief Abstract plugin.
 *
 * A plugin is identified by name and can be loaded and unloaded at runtime.
 */
class plugin : public std::enable_shared_from_this<plugin> {
private:
    // Plugin information
    std::string name_;
    std::string path_;

    // Metadata
    std::string author_{"unknown"};
    std::string license_{"unknown"};
    std::string summary_{"unknown"};
    std::string version_{"unknown"};

public:
    /**
     * Constructor.
     *
     * \param name the plugin id
     * \param path the fully resolved path to the plugin
     * \throws std::runtime_error on errors
     */
    inline plugin(std::string name, std::string path) noexcept
        : name_(std::move(name))
        , path_(std::move(path))
    {
    }

    /**
     * Temporary, close all timers.
     */
    virtual ~plugin() = default;

    /**
     * Get the plugin name.
     *
     * \return the plugin name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Get the plugin path.
     *
     * \return the plugin path
     * \note some plugins may not exist on the disk
     */
    inline const std::string& path() const noexcept
    {
        return path_;
    }

    /**
     * Get the author.
     *
     * \return the author
     */
    inline const std::string& author() const noexcept
    {
        return author_;
    }

    /**
     * Set the author.
     *
     * \param author the author
     */
    inline void set_author(std::string author) noexcept
    {
        author_ = std::move(author);
    }

    /**
     * Get the license.
     *
     * \return the license
     */
    inline const std::string& license() const noexcept
    {
        return license_;
    }

    /**
     * Set the license.
     *
     * \param license the license
     */
    inline void set_license(std::string license) noexcept
    {
        license_ = std::move(license);
    }

    /**
     * Get the summary.
     *
     * \return the summary
     */
    inline const std::string& summary() const noexcept
    {
        return summary_;
    }

    /**
     * Set the summary.
     *
     * \param summary the summary
     */
    inline void set_summary(std::string summary) noexcept
    {
        summary_ = std::move(summary);
    }

    /**
     * Get the version.
     *
     * \return the version
     */
    inline const std::string& version() const noexcept
    {
        return version_;
    }

    /**
     * Set the version.
     *
     * \param version the version
     */
    inline void set_version(std::string version) noexcept
    {
        version_ = std::move(version);
    }

    /**
     * Access the plugin configuration.
     *
     * \return the config
     */
    virtual plugin_config config()
    {
        return {};
    }

    /**
     * Set the configuration.
     *
     * \param config the configuration
     */
    virtual void set_config(plugin_config config)
    {
        util::unused(config);
    }

    /**
     * Access the plugin formats.
     *
     * \return the format
     */
    virtual plugin_formats formats()
    {
        return {};
    }

    /**
     * Set the formats.
     *
     * \param formats the formats
     */
    virtual void set_formats(plugin_formats formats)
    {
        util::unused(formats);
    }

    /**
     * Access the plugin paths.
     *
     * \return the paths
     */
    virtual plugin_paths paths()
    {
        return {};
    }

    /**
     * Set the paths.
     *
     * \param paths the paths
     */
    virtual void set_paths(plugin_paths paths)
    {
        util::unused(paths);
    }

    /**
     * On channel message. This event will call onMessage or
     * onCommand if the messages starts with the command character
     * plus the plugin name.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_command(irccd& irccd, const message_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On successful connection.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_connect(irccd& irccd, const connect_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On invitation.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_invite(irccd& irccd, const invite_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On join.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_join(irccd& irccd, const join_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On kick.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_kick(irccd& irccd, const kick_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On load.
     *
     * \param irccd the irccd instance
     */
    virtual void on_load(irccd& irccd)
    {
        util::unused(irccd);
    }

    /**
     * On channel message.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_message(irccd& irccd, const message_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On CTCP Action.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_me(irccd& irccd, const me_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user mode change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_mode(irccd& irccd, const mode_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On names listing.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_names(irccd& irccd, const names_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On nick change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_nick(irccd& irccd, const nick_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user notice.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_notice(irccd& irccd, const notice_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On part.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_part(irccd& irccd, const part_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user query.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_query(irccd& irccd, const query_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user query command.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_query_command(irccd& irccd, const query_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On reload.
     *
     * \param irccd the irccd instance
     */
    virtual void on_reload(irccd& irccd)
    {
        util::unused(irccd);
    }

    /**
     * On topic change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_topic(irccd& irccd, const topic_event& event)
    {
        util::unused(irccd, event);
    }

    /**
     * On unload.
     *
     * \param irccd the irccd instance
     */
    virtual void on_unload(irccd& irccd)
    {
        util::unused(irccd);
    }

    /**
     * On whois information.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void on_whois(irccd& irccd, const whois_event& event)
    {
        util::unused(irccd, event);
    }
};

/**
 * \brief Abstract interface for searching plugins.
 *
 * This class is used to make loading of plugins extensible, the plugin_service
 * knows some predefined plugins loaders and use them to search for available
 * plugins.
 *
 * This makes easier to implement new plugins or new ways of loading them.
 *
 * \see dynlib_plugin_loader
 * \see js_plugin_loader
 */
class plugin_loader {
private:
    std::vector<std::string> directories_;
    std::vector<std::string> extensions_;

public:
    /**
     * Construct the loader with a predefined set of directories and extensions.
     *
     * If directories is not specified, a sensible default list of system and
     * user paths are searched.
     *
     * If extensions is empty, default find function implementation does
     * nothing.
     *
     * \param directories directories to search
     * \param extensions the list of extensions supported
     */
    plugin_loader(std::vector<std::string> directories = {},
                  std::vector<std::string> extensions = {});

    /**
     * Set directories where to search plugins.
     *
     * \param dirs the directories
     */
    inline void set_directories(std::vector<std::string> dirs)
    {
        directories_ = std::move(dirs);
    }

    /**
     * Set supported extensions for this loader.
     *
     * \param extensions the extensions (with the dot)
     */
    inline void set_extensions(std::vector<std::string> extensions)
    {
        extensions_ = std::move(extensions);
    }

    /**
     * Try to open the plugin specified by path.
     *
     * The implementation must test if the plugin is suitable for opening, by
     * testing extension for example.
     *
     * \param file the file
     */
    virtual std::shared_ptr<plugin> open(const std::string& id,
                                         const std::string& file) noexcept = 0;

    /**
     * Search for a plugin named by this id.
     *
     * \param id the plugin id
     * \return the plugin
     */
    virtual std::shared_ptr<plugin> find(const std::string& id) noexcept;
};

/**
 * \brief Plugin error.
 */
class plugin_error : public boost::system::system_error {
public:
    /**
     * \brief Server related errors (3000..3999)
     */
    enum error {
        //!< No error.
        no_error = 0,

        //!< The specified plugin is not found.
        not_found = 2000,

        //!< The plugin was unable to run the function.
        exec_error,

        //!< The plugin is already loaded.
        already_exists,
    };

    /**
     * Inherited constructors.
     */
    using system_error::system_error;
};

/**
 * Get the plugin error category singleton.
 *
 * \return the singleton
 */
const boost::system::error_category& server_category();

/**
 * Create a boost::system::error_code from plugin_error::error enum.
 *
 * \param e the error code
 */
boost::system::error_code make_error_code(plugin_error::error e);

} // !irccd

namespace boost {

namespace system {

template <>
struct is_error_code_enum<irccd::plugin_error::error> : public std::true_type {
};

} // !system

} // !boost

#endif // !IRCCD_PLUGIN_HPP
