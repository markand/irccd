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

#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <vector>

namespace irccd {

class irccd;

struct connect_event;
struct disconnect_event;
struct invite_event;
struct join_event;
struct kick_event;
struct me_event;
struct message_event;
struct mode_event;
struct names_event;
struct nick_event;
struct notice_event;
struct part_event;
struct topic_event;
struct whois_event;

/**
 * \ingroup plugins
 * \brief Abstract plugin.
 *
 * A plugin is identified by name and can be loaded and unloaded at runtime.
 */
class plugin : public std::enable_shared_from_this<plugin> {
public:
    /**
     * Map for key/value pairs.
     *
     * Used in options, formats and paths.
     */
    using map = std::unordered_map<std::string, std::string>;

    /**
     * Temporary, close all timers.
     */
    virtual ~plugin() = default;

    /**
     * Get the plugin name.
     *
     * \return the plugin name
     */
    virtual auto get_name() const noexcept -> std::string_view = 0;

    /**
     * Get the author.
     *
     * \return the author
     */
    virtual auto get_author() const noexcept -> std::string_view
    {
        return "unknown";
    }

    /**
     * Get the license.
     *
     * \return the license
     */
    virtual auto get_license() const noexcept -> std::string_view
    {
        return "unknown";
    }

    /**
     * Get the summary.
     *
     * \return the summary
     */
    virtual auto get_summary() const noexcept -> std::string_view
    {
        return "unknown";
    }

    /**
     * Get the version.
     *
     * \return the version
     */
    virtual auto get_version() const noexcept -> std::string_view
    {
        return "unknown";
    }

    /**
     * Get all options.
     *
     * \return options
     */
    virtual auto get_options() const -> map
    {
        return {};
    }

    /**
     * Set all options.
     *
     * \param map the options
     */
    virtual void set_options(const map& map)
    {
        (void)map;
    }

    /**
     * Get all formats.
     *
     * \return formats
     */
    virtual auto get_formats() const -> map
    {
        return {};
    }

    /**
     * Set all formats.
     *
     * \param map the formats
     */
    virtual void set_formats(const map& map)
    {
        (void)map;
    }

    /**
     * Get all paths.
     *
     * \return paths
     */
    virtual auto get_paths() const -> map
    {
        return {};
    }

    /**
     * Set all paths.
     *
     * \param map the paths
     */
    virtual void set_paths(const map& map)
    {
        (void)map;
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
    plugin_loader(std::vector<std::string> directories,
                  std::vector<std::string> extensions) noexcept;

    /**
     * Try to open the plugin specified by path.
     *
     * The implementation must test if the plugin is suitable for opening, by
     * testing extension for example.
     *
     * \param id the plugin identifier
     * \param file the file path
     * \throw plugin_error on errors
     */
    virtual auto open(std::string_view id, std::string_view file) -> std::shared_ptr<plugin> = 0;

    /**
     * Search for a plugin named by this id.
     *
     * \param id the plugin id
     * \return the plugin
     * \throw plugin_error on errors
     */
    virtual auto find(std::string_view id) -> std::shared_ptr<plugin>;
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
    plugin_error(error code, std::string_view name = "", std::string_view message = "");

    /**
     * Get the plugin name.
     *
     * \return the name
     */
    auto get_name() const noexcept -> const std::string&;

    /**
     * Get the additional message.
     *
     * \return the message
     */
    auto get_message() const noexcept -> const std::string&;

    /**
     * Get message appropriate for use with logger.
     */
    auto what() const noexcept -> const char* override;
};

/**
 * Get the plugin error category singleton.
 *
 * \return the singleton
 */
auto plugin_category() -> const std::error_category&;

/**
 * Create a boost::system::error_code from plugin_error::error enum.
 *
 * \param e the error code
 */
auto make_error_code(plugin_error::error e) -> std::error_code ;

} // !irccd

namespace std {

template <>
struct is_error_code_enum<irccd::plugin_error::error> : public std::true_type {
};

} // !std

#endif // !IRCCD_DAEMON_PLUGIN_HPP
