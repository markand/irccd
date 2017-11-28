/*
 * command.hpp -- remote command
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

#ifndef IRCCD_COMMAND_HPP
#define IRCCD_COMMAND_HPP

/**
 * \file command.hpp
 * \brief Remote commands.
 */

#include <cassert>
#include <map>
#include <vector>

#include "json.hpp"
#include "sysconfig.hpp"

namespace irccd {

class irccd;
class irccdctl;
class transport_client;

/**
 * \brief Server side remote command
 */
class command {
private:
    std::string name_;

public:
    /**
     * Construct a command.
     *
     * \pre !name.empty()
     * \param name the command name
     */
    inline command(std::string name) noexcept
        : name_(std::move(name))
    {
        assert(!name_.empty());
    }

    /**
     * Default destructor virtual.
     */
    virtual ~command() = default;

    /**
     * Return the command name, must not have spaces.
     *
     * \return the command name
     */
    inline const std::string& name() const noexcept
    {
        return name_;
    }

    /**
     * Execute the command.
     *
     * If the command throw an exception, the error is sent to the client so be
     * careful about sensitive information.
     *
     * The implementation should use client.success() or client.error() to send
     * some data.
     *
     * \param irccd the irccd instance
     * \param client the client
     * \param args the client arguments
     */
    virtual void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) = 0;
};

/**
 * \brief Implementation of plugin-config transport command.
 *
 * Replies:
 *
 *   - plugin_error::not_found
 */
class plugin_config_command : public command {
public:
    /**
     * Constructor.
     */
    plugin_config_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of plugin-info transport command.
 *
 * Replies:
 *
 *   - plugin_error::not_found
 */
class plugin_info_command : public command {
public:
    /**
     * Constructor.
     */
    plugin_info_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of plugin-list transport command.
 */
class plugin_list_command : public command {
public:
    /**
     * Constructor.
     */
    plugin_list_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& request) override;
};

/**
 * \brief Implementation of plugin-load transport command.
 *
 * Replies:
 *
 *   - plugin_error::already_exists
 *   - plugin_error::not_found
 *   - pluign_error::exec_error
 */
class plugin_load_command : public command {
public:
    /**
     * Constructor.
     */
    plugin_load_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of plugin-reload transport command.
 *
 * Replies:
 *
 *   - plugin_error::not_found
 *   - pluign_error::exec_error
 */
class plugin_reload_command : public command {
public:
    /**
     * Constructor.
     */
    plugin_reload_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of plugin-unload transport command.
 *
 * Replies:
 *
 *   - plugin_error::not_found
 *   - pluign_error::exec_error
 */
class plugin_unload_command : public command {
public:
    /**
     * Constructor.
     */
    plugin_unload_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-cmode transport command.
 */
class server_channel_mode_command : public command {
public:
    /**
     * Constructor.
     */
    server_channel_mode_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-cnotice transport command.
 */
class server_channel_notice_command : public command {
public:
    /**
     * Constructor.
     */
    server_channel_notice_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-connect transport command.
 */
class server_connect_command : public command {
public:
    /**
     * Constructor.
     */
    server_connect_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-disconnect transport command.
 */
class server_disconnect_command : public command {
public:
    /**
     * Constructor.
     */
    server_disconnect_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-info transport command.
 */
class server_info_command : public command {
public:
    /**
     * Constructor.
     */
    server_info_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-invite transport command.
 */
class server_invite_command : public command {
public:
    /**
     * Constructor.
     */
    server_invite_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-join transport command.
 */
class server_join_command : public command {
public:
    /**
     * Constructor.
     */
    server_join_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-kick transport command.
 */
class server_kick_command : public command {
public:
    /**
     * Constructor.
     */
    server_kick_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-list transport command.
 */
class server_list_command : public command {
public:
    /**
     * Constructor.
     */
    server_list_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-me transport command.
 */
class server_me_command : public command {
public:
    /**
     * Constructor.
     */
    server_me_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-message transport command.
 */
class server_message_command : public command {
public:
    /**
     * Constructor.
     */
    server_message_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-mode transport command.
 */
class server_mode_command : public command {
public:
    /**
     * Constructor.
     */
    server_mode_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-nick transport command.
 */
class server_nick_command : public command {
public:
    /**
     * Constructor.
     */
    server_nick_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-notice transport command.
 */
class server_notice_command : public command {
public:
    /**
     * Constructor.
     */
    server_notice_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-part transport command.
 */
class server_part_command : public command {
public:
    /**
     * Constructor.
     */
    server_part_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-reconnect transport command.
 */
class server_reconnect_command : public command {
public:
    /**
     * Constructor.
     */
    server_reconnect_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of server-topic transport command.
 */
class server_topic_command : public command {
public:
    /**
     * Constructor.
     */
    server_topic_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of rule-edit transport command.
 */
class rule_edit_command : public command {
public:
    /**
     * Constructor.
     */
    rule_edit_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of rule-list transport command.
 */
class rule_list_command : public command {
public:
    /**
     * Constructor.
     */
    rule_list_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of rule-info transport command.
 */
class rule_info_command : public command {
public:
    /**
     * Constructor.
     */
    rule_info_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of rule-remove transport command.
 */
class rule_remove_command : public command {
public:
    /**
     * Constructor.
     */
    rule_remove_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of rule-move transport command.
 */
class rule_move_command : public command {
public:
    /**
     * Constructor.
     */
    rule_move_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

/**
 * \brief Implementation of rule-add transport command.
 */
class rule_add_command : public command {
public:
    /**
     * Constructor.
     */
    rule_add_command();

    /**
     * \copydoc command::exec
     */
    void exec(irccd& irccd, transport_client& client, const nlohmann::json& args) override;
};

} // !irccd

#endif // !IRCCD_COMMAND_HPP
