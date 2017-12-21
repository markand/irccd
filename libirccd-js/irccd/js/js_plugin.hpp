/*
 * js_plugin.hpp -- JavaScript plugins for irccd
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

#ifndef IRCCD_JS_PLUGIN_HPP
#define IRCCD_JS_PLUGIN_HPP

/**
 * \file js_plugin.hpp
 * \brief JavaScript plugins for irccd.
 */

#include <vector>

#include <irccd/daemon/plugin.hpp>

#include "duktape.hpp"

namespace irccd {

class jsapi;

/**
 * \brief JavaScript plugins for irccd.
 * \ingroup plugins
 */
class js_plugin : public plugin {
public:
    /**
     * Global property where to read/write plugin configuration (object).
     */
    static const std::string config_property;

    /**
     * Global property where to read/write plugin formats (object).
     */
    static const std::string format_property;

    /**
     * Global property where paths are defined (object).
     */
    static const std::string paths_property;

private:
    // JavaScript context
    dukx_context context_;

    // Private helpers.
    std::unordered_map<std::string, std::string> get_table(const std::string&) const;
    void put_table(const std::string&, const std::unordered_map<std::string, std::string>&);

    /*
     * Helpers to call a Javascript function.
     */
    void push() noexcept;

    template <typename Value, typename... Args>
    void push(Value&& value, Args&&... args);

    template <typename... Args>
    void call(const std::string&, Args&&... args);

public:
    /**
     * Constructor.
     *
     * \param name the plugin name
     * \param path the path to the plugin
     */
    js_plugin(std::string name, std::string path);

    /**
     * Access the Duktape context.
     *
     * \return the context
     */
    inline dukx_context& context() noexcept
    {
        return context_;
    }

    /**
     * Open the script file associated.
     */
    void open();

    /**
     * \copydoc Plugin::config
     */
    plugin_config config() override
    {
        return get_table(config_property);
    }

    /**
     * \copydoc Plugin::setConfig
     */
    void set_config(plugin_config config) override
    {
        put_table(config_property, config);
    }

    /**
     * \copydoc Plugin::formats
     */
    plugin_formats formats() override
    {
        return get_table(format_property);
    }

    /**
     * \copydoc Plugin::setFormats
     */
    void set_formats(plugin_formats formats) override
    {
        put_table(format_property, formats);
    }

    /**
     * \copydoc Plugin::paths
     */
    plugin_paths paths() override
    {
        return get_table(paths_property);
    }

    /**
     * \copydoc Plugin::set_paths
     */
    void set_paths(plugin_paths paths) override
    {
        put_table(paths_property, std::move(paths));
    }

    /**
     * \copydoc Plugin::on_command
     */
    void on_command(irccd& irccd, const message_event& event) override;

    /**
     * \copydoc Plugin::on_connect
     */
    void on_connect(irccd& irccd, const connect_event& event) override;

    /**
     * \copydoc Plugin::on_invite
     */
    void on_invite(irccd& irccd, const invite_event& event) override;

    /**
     * \copydoc Plugin::on_join
     */
    void on_join(irccd& irccd, const join_event& event) override;

    /**
     * \copydoc Plugin::on_kick
     */
    void on_kick(irccd& irccd, const kick_event& event) override;

    /**
     * \copydoc Plugin::on_load
     */
    void on_load(irccd& irccd) override;

    /**
     * \copydoc Plugin::on_message
     */
    void on_message(irccd& irccd, const message_event& event) override;

    /**
     * \copydoc Plugin::on_me
     */
    void on_me(irccd& irccd, const me_event& event) override;

    /**
     * \copydoc Plugin::on_mode
     */
    void on_mode(irccd& irccd, const mode_event& event) override;

    /**
     * \copydoc Plugin::on_names
     */
    void on_names(irccd& irccd, const names_event& event) override;

    /**
     * \copydoc Plugin::on_nick
     */
    void on_nick(irccd& irccd, const nick_event& event) override;

    /**
     * \copydoc Plugin::on_notice
     */
    void on_notice(irccd& irccd, const notice_event& event) override;

    /**
     * \copydoc Plugin::on_part
     */
    void on_part(irccd& irccd, const part_event& event) override;

    /**
     * \copydoc Plugin::on_reload
     */
    void on_reload(irccd& irccd) override;

    /**
     * \copydoc Plugin::on_topic
     */
    void on_topic(irccd& irccd, const topic_event& event) override;

    /**
     * \copydoc Plugin::on_unload
     */
    void on_unload(irccd& irccd) override;

    /**
     * \copydoc Plugin::on_whois
     */
    void on_whois(irccd& irccd, const whois_event& event) override;
};

/**
 * \brief Implementation for searching Javascript plugins.
 */
class js_plugin_loader : public plugin_loader {
public:
    using modules_t = std::vector<std::unique_ptr<jsapi>>;

private:
    irccd& irccd_;
    modules_t modules_;

public:
    /**
     * Create a plugin_loader with all irccd Javascript modules.
     *
     * \param irccd the irccd instance
     * \return a ready to use plugin_loader
     */
    static std::unique_ptr<js_plugin_loader> defaults(irccd& irccd);

    /**
     * Constructor.
     *
     * \param irccd the irccd instance
     */
    js_plugin_loader(irccd& irccd) noexcept;

    /**
     * Destructor defaulted.
     */
    ~js_plugin_loader() noexcept;

    /**
     * Get the list of modules.
     *
     * \return the modules
     */
    inline const modules_t& modules() const noexcept
    {
        return modules_;
    }

    /**
     * Overloaded function.
     *
     * \return the modules
     */
    inline modules_t& modules() noexcept
    {
        return modules_;
    }

    /**
     * \copydoc PluginLoader::open
     */
    std::shared_ptr<plugin> open(const std::string& id,
                                 const std::string& path) override;
};

template <>
class dukx_type_traits<whois> : public std::true_type {
public:
    static void push(duk_context* ctx, const whois& who);
};

} // !irccd

#endif // !IRCCD_PLUGIN_JS_HPP
