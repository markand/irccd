/*
 * dynlib_plugin.hpp -- native plugin implementation
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

#ifndef IRCCD_DAEMON_DYNLIB_PLUGIN_HPP
#define IRCCD_DAEMON_DYNLIB_PLUGIN_HPP

/**
 * \file plugin-dynlib.hpp
 * \brief Native plugin implementation.
 */

#include <boost/dll.hpp>

#include "plugin.hpp"

namespace irccd {

/**
 * \brief Dynlib based plugin.
 * \ingroup plugins
 */
class dynlib_plugin : public plugin {
private:
    boost::dll::shared_library dso_;
    std::unique_ptr<plugin> plugin_;

public:
    /**
     * Construct the plugin.
     *
     * \param name the name
     * \param path the fully resolved path (must be absolute)
     * \throw std::exception on failures
     */
    dynlib_plugin(std::string name, std::string path);

    /**
     * \copydoc plugin::on_command
     */
    void on_command(irccd& irccd, const message_event& event) override;

    /**
     * \copydoc plugin::on_connect
     */
    void on_connect(irccd& irccd, const connect_event& event) override;

    /**
     * \copydoc plugin::on_invite
     */
    void on_invite(irccd& irccd, const invite_event& event) override;

    /**
     * \copydoc plugin::on_join
     */
    void on_join(irccd& irccd, const join_event& event) override;

    /**
     * \copydoc plugin::on_kick
     */
    void on_kick(irccd& irccd, const kick_event& event) override;

    /**
     * \copydoc plugin::on_load
     */
    void on_load(irccd& irccd) override;

    /**
     * \copydoc plugin::on_message
     */
    void on_message(irccd& irccd, const message_event& event) override;

    /**
     * \copydoc plugin::on_me
     */
    void on_me(irccd& irccd, const me_event& event) override;

    /**
     * \copydoc plugin::on_mode
     */
    void on_mode(irccd& irccd, const mode_event& event) override;

    /**
     * \copydoc plugin::on_names
     */
    void on_names(irccd& irccd, const names_event& event) override;

    /**
     * \copydoc plugin::on_nick
     */
    void on_nick(irccd& irccd, const nick_event& event) override;

    /**
     * \copydoc plugin::on_notice
     */
    void on_notice(irccd& irccd, const notice_event& event) override;

    /**
     * \copydoc plugin::on_part
     */
    void on_part(irccd& irccd, const part_event& event) override;

    /**
     * \copydoc plugin::on_reload
     */
    void on_reload(irccd& irccd) override;

    /**
     * \copydoc plugin::on_topic
     */
    void on_topic(irccd& irccd, const topic_event& event) override;

    /**
     * \copydoc plugin::on_unload
     */
    void on_unload(irccd& irccd) override;

    /**
     * \copydoc plugin::on_whois
     */
    void on_whois(irccd& irccd, const whois_event& event) override;
};

/**
 * \brief Implementation for searching native plugins.
 */
class dynlib_plugin_loader : public plugin_loader {
public:
    /**
     * Constructor.
     *
     * \param directories optional directories to search, if empty use defaults.
     */
    dynlib_plugin_loader(std::vector<std::string> directories = {}) noexcept;

    /**
     * \copydoc plugin_loader::find
     */
    std::shared_ptr<plugin> open(const std::string& id,
                                 const std::string& path) noexcept override;
};

} // !irccd

#endif // !IRCCD_DAEMON_DYNLIB_PLUGIN_HPP
