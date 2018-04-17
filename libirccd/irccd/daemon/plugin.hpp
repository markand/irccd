/*
 * plugin.hpp -- irccd JavaScript plugin interface
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

#ifndef IRCCD_DAEMON_PLUGIN_HPP
#define IRCCD_DAEMON_PLUGIN_HPP

/**
 * \file plugin.hpp
 * \brief irccd plugins
 */

/**
 * \defgroup plugins Plugins
 * \brief Plugin management.
 */

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <memory>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace irccd {

class irccd;

class connect_event;
class disconnect_event;
class invite_event;
class join_event;
class kick_event;
class me_event;
class message_event;
class mode_event;
class names_event;
class nick_event;
class notice_event;
class part_event;
class topic_event;
class whois_event;

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
    inline const std::string& get_name() const noexcept
    {
        return name_;
    }

    /**
     * Get the plugin path.
     *
     * \return the plugin path
     * \note some plugins may not exist on the disk
     */
    inline const std::string& get_path() const noexcept
    {
        return path_;
    }

    /**
     * Get the author.
     *
     * \return the author
     */
    inline const std::string& get_author() const noexcept
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
    inline const std::string& get_license() const noexcept
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
    inline const std::string& get_summary() const noexcept
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
    inline const std::string& get_version() const noexcept
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
    virtual plugin_config get_config()
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
        (void)config;
    }

    /**
     * Access the plugin formats.
     *
     * \return the format
     */
    virtual plugin_formats get_formats()
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
        (void)formats;
    }

    /**
     * Access the plugin paths.
     *
     * \return the paths
     */
    virtual plugin_paths get_paths()
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
        (void)paths;
    }

    /**
     * On channel message. This event will call onMessage or
     * onCommand if the messages starts with the command character
     * plus the plugin name.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_command(irccd& irccd, const message_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On successful connection.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_connect(irccd& irccd, const connect_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On disconnection.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_disconnect(irccd& irccd, const disconnect_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On invitation.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_invite(irccd& irccd, const invite_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On join.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_join(irccd& irccd, const join_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On kick.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_kick(irccd& irccd, const kick_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On load.
     *
     * \param irccd the irccd instance
     */
    virtual void handle_load(irccd& irccd)
    {
        (void)irccd;
    }

    /**
     * On channel message.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_message(irccd& irccd, const message_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On CTCP Action.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_me(irccd& irccd, const me_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On user mode change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_mode(irccd& irccd, const mode_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On names listing.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_names(irccd& irccd, const names_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On nick change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_nick(irccd& irccd, const nick_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On user notice.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_notice(irccd& irccd, const notice_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On part.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_part(irccd& irccd, const part_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On reload.
     *
     * \param irccd the irccd instance
     */
    virtual void handle_reload(irccd& irccd)
    {
        (void)irccd;
    }

    /**
     * On topic change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_topic(irccd& irccd, const topic_event& event)
    {
        (void)irccd;
        (void)event;
    }

    /**
     * On unload.
     *
     * \param irccd the irccd instance
     */
    virtual void handle_unload(irccd& irccd)
    {
        (void)irccd;
    }

    /**
     * On whois information.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void handle_whois(irccd& irccd, const whois_event& event)
    {
        (void)irccd;
        (void)event;
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
     * \pre !extensions.empty()
     * \param directories optional list of directories to search
     * \param extensions the non empty list of extensions supported
     */
    inline plugin_loader(std::vector<std::string> directories,
                  std::vector<std::string> extensions) noexcept
        : directories_(std::move(directories))
        , extensions_(std::move(extensions))
    {
        assert(!extensions_.empty());
    }

    /**
     * Set directories where to search plugins.
     *
     * \param directories the directories
     */
    inline void set_directories(std::vector<std::string> directories)
    {
        directories_ = std::move(directories);
    }

    /**
     * Set supported extensions for this loader.
     *
     * \pre !extensions.empty()
     * \param extensions the extensions (with the dot)
     */
    inline void set_extensions(std::vector<std::string> extensions)
    {
        assert(!extensions.empty());

        extensions_ = std::move(extensions);
    }

    /**
     * Try to open the plugin specified by path.
     *
     * The implementation must test if the plugin is suitable for opening, by
     * testing extension for example.
     *
     * \param id the plugin identifier
     * \param file the file path
     */
    virtual std::shared_ptr<plugin> open(const std::string& id,
                                         const std::string& file) = 0;

    /**
     * Search for a plugin named by this id.
     *
     * \param id the plugin id
     * \return the plugin
     */
    virtual std::shared_ptr<plugin> find(const std::string& id);
};

/**
 * \brief Plugin error.
 */
class plugin_error : public std::system_error {
public:
    /**
     * \brief Plugin related errors.
     */
    enum error {
        //!< No error.
        no_error = 0,

        //!< The specified identifier is invalid.
        invalid_identifier,

        //!< The specified plugin is not found.
        not_found,

        //!< The plugin was unable to run the function.
        exec_error,

        //!< The plugin is already loaded.
        already_exists,
    };

private:
    std::string name_;
    std::string message_;
    std::string what_;

public:
    /**
     * Constructor.
     *
     * \param code the error code
     * \param name the plugin name
     * \param message the optional message (e.g. error from plugin)
     */
    plugin_error(error code, std::string name = "", std::string message = "") noexcept;

    /**
     * Get the plugin name.
     *
     * \return the name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Get the additional message.
     *
     * \return the message
     */
    inline const std::string& message() const noexcept
    {
        return message_;
    }

    /**
     * Get message appropriate for use with logger.
     */
    const char* what() const noexcept override
    {
        return what_.c_str();
    }
};

/**
 * Get the plugin error category singleton.
 *
 * \return the singleton
 */
const std::error_category& plugin_category();

/**
 * Create a boost::system::error_code from plugin_error::error enum.
 *
 * \param e the error code
 */
std::error_code make_error_code(plugin_error::error e);

} // !irccd

namespace std {

template <>
struct is_error_code_enum<irccd::plugin_error::error> : public std::true_type {
};

} // !std

#endif // !IRCCD_DAEMON_PLUGIN_HPP
