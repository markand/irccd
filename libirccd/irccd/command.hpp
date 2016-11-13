/*
 * command.hpp -- remote command
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

class Irccd;
class Irccdctl;
class TransportClient;

/**
 * \brief Server side remote command
 */
class Command {
private:
    std::string m_name;

public:
    /**
     * Construct a command.
     *
     * \pre !name.empty()
     * \param name the command name
     */
    inline Command(std::string name) noexcept
        : m_name(std::move(name))
    {
        assert(!m_name.empty());
    }

    /**
     * Default destructor virtual.
     */
    virtual ~Command() = default;

    /**
     * Return the command name, must not have spaces.
     *
     * \return the command name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
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
    IRCCD_EXPORT virtual void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) = 0;
};

namespace command {

/**
 * \brief Implementation of plugin-config transport command.
 */
class PluginConfigCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginConfigCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of plugin-info transport command.
 */
class PluginInfoCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginInfoCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of plugin-list transport command.
 */
class PluginListCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginListCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &request) override;
};

/**
 * \brief Implementation of plugin-load transport command.
 */
class PluginLoadCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginLoadCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of plugin-reload transport command.
 */
class PluginReloadCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginReloadCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of plugin-unload transport command.
 */
class PluginUnloadCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT PluginUnloadCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-cmode transport command.
 */
class ServerChannelModeCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerChannelModeCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-cnotice transport command.
 *
 * Send a channel notice to the specified channel.
 *
 * {
 *   "command": "server-cnotice",
 *   "server": "the server name",
 *   "channel": "name",
 *   "message": "the message"
 * }
 */
class ServerChannelNoticeCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerChannelNoticeCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-connect transport command.
 */
class ServerConnectCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerConnectCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-disconnect transport command.
 */
class IRCCD_EXPORT ServerDisconnectCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerDisconnectCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-info transport command.
 */
class IRCCD_EXPORT ServerInfoCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerInfoCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-invite transport command.
 */
class ServerInviteCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerInviteCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-join transport command.
 */
class ServerJoinCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerJoinCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-kick transport command.
 */
class ServerKickCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerKickCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-list transport command.
 */
class ServerListCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerListCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-me transport command.
 */
class IRCCD_EXPORT ServerMeCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerMeCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-message transport command.
 */
class IRCCD_EXPORT ServerMessageCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerMessageCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-mode transport command.
 */
class IRCCD_EXPORT ServerModeCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerModeCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-nick transport command.
 */
class IRCCD_EXPORT ServerNickCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerNickCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-notice transport command.
 */
class IRCCD_EXPORT ServerNoticeCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerNoticeCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \class ServerPart
 * \brief Implementation of server-part transport command.
 */
class IRCCD_EXPORT ServerPartCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerPartCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-reconnect transport command.
 */
class ServerReconnectCommand : public Command {
public:
    /**
     * Constructor.
     */
    IRCCD_EXPORT ServerReconnectCommand();

    /**
     * \copydoc Command::exec
     */
    IRCCD_EXPORT void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

/**
 * \brief Implementation of server-topic transport command.
 */
class IRCCD_EXPORT ServerTopicCommand : public Command {
public:
    /**
     * Constructor.
     */
    ServerTopicCommand();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args) override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_SERVER_TOPIC_HPP
