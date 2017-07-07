/*
 * cli.hpp -- command line for irccdctl
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

#ifndef IRCCD_CLI_HPP
#define IRCCD_CLI_HPP

#include <string>
#include <vector>

#include <json.hpp>

namespace irccd {

class Irccdctl;

/*
 * Cli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Abstract CLI class.
 */
class Cli {
protected:
    std::string m_name;
    std::string m_summary;
    std::string m_usage;
    std::string m_help;

    /**
     * Check the message result and print an error if any.
     *
     * \param result the result
     * \throw an error if any
     */
    void check(const nlohmann::json &result);

    /**
     * Send a request and wait for the response.
     *
     * \param irccdctl the client
     * \param args the optional arguments
     */
    nlohmann::json request(Irccdctl &irccdctl, nlohmann::json args = nullptr);

    /**
     * Call a command and wait for success/error response.
     *
     * \param irccdctl the client
     * \param args the extra arguments (may be null)
     */
    void call(Irccdctl &irccdctl, nlohmann::json args);

public:
    inline Cli(std::string name, std::string summary, std::string usage, std::string help) noexcept
        : m_name(std::move(name))
        , m_summary(std::move(summary))
        , m_usage(std::move(usage))
        , m_help(std::move(help))
    {
    }

    virtual ~Cli() = default;

    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    inline const std::string &summary() const noexcept
    {
        return m_summary;
    }

    inline const std::string &usage() const noexcept
    {
        return m_usage;
    }

    inline const std::string &help() const noexcept
    {
        return m_help;
    }

    virtual void exec(Irccdctl &client, const std::vector<std::string> &args) = 0;
};

namespace cli {

/*
 * PluginConfigCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl plugin-config.
 */
class PluginConfigCli : public Cli {
private:
    void set(Irccdctl &, const std::vector<std::string> &);
    void get(Irccdctl &, const std::vector<std::string> &);
    void getall(Irccdctl &, const std::vector<std::string> &);

public:
    /**
     * Default constructor.
     */
    PluginConfigCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * PluginInfoCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl plugin-info.
 */
class PluginInfoCli : public Cli {
public:
    /**
     * Default constructor.
     */
    PluginInfoCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * PluginListCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl plugin-list.
 */
class PluginListCli : public Cli {
public:
    /**
     * Default constructor.
     */
    PluginListCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * PluginLoadCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl plugin-load.
 */
class PluginLoadCli : public Cli {
public:
    /**
     * Default constructor.
     */
    PluginLoadCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * PluginReloadCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl plugin-reload.
 */
class PluginReloadCli : public Cli {
public:
    PluginReloadCli();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * PluginUnloadCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl plugin-unload.
 */
class PluginUnloadCli : public Cli {
public:
    PluginUnloadCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerChannelCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-cmode.
 */
class ServerChannelMode : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerChannelMode();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerChannelNoticeCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-cnotice.
 */
class ServerChannelNoticeCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerChannelNoticeCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerConnectCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-connect.
 */
class ServerConnectCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerConnectCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerDisconnectCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-disconnect.
 */
class ServerDisconnectCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerDisconnectCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerInfoCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-info.
 */
class ServerInfoCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerInfoCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerInviteCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-invite.
 */
class ServerInviteCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerInviteCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerJoinCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-join.
 */
class ServerJoinCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerJoinCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerKickCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-kick.
 */
class ServerKickCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerKickCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerListCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-list.
 */
class ServerListCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerListCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerMeCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-me.
 */
class ServerMeCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerMeCli();

    /**
     * \copydoc Command::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerMessageCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-message.
 */
class ServerMessageCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerMessageCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerModeCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-mode.
 */
class ServerModeCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerModeCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerNickCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-nick.
 */
class ServerNickCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerNickCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerNoticeCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-notice.
 */
class ServerNoticeCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerNoticeCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerPartCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-part.
 */
class ServerPartCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerPartCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerReconnectCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-reconnect.
 */
class ServerReconnectCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerReconnectCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) override;
};

/*
 * ServerTopicCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl server-topic.
 */
class ServerTopicCli : public Cli {
public:
    /**
     * Default constructor.
     */
    ServerTopicCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &client, const std::vector<std::string> &args) override;
};

/*
 * RuleListCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl rule-list.
 */
class RuleListCli : public Cli {
public:
    /**
     * Default constructor.
     */
    RuleListCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &client, const std::vector<std::string> &args) override;
};

/*
 * RuleInfoCli
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl rule-info.
 */
class RuleInfoCli : public Cli {
public:
    /**
     * Default constructor.
     */
    RuleInfoCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &client, const std::vector<std::string> &args) override;
};

/*
 * RuleRemoveCli
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl rule-remove.
 */
class RuleRemoveCli : public Cli {
public:
    /**
     * Default constructor.
     */
    RuleRemoveCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &client, const std::vector<std::string> &args) override;
};

/*
 * RuleMoveCli
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl rule-move.
 */
class RuleMoveCli : public Cli {
public:
    /**
     * Default constructor.
     */
    RuleMoveCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &client, const std::vector<std::string> &args) override;
};

/*
 * WatchCli.
 * ------------------------------------------------------------------
 */

/**
 * \brief Implementation of irccdctl watch.
 */
class WatchCli : public Cli {
public:
    /**
     * Default constructor.
     */
    WatchCli();

    /**
     * \copydoc Cli::exec
     */
    void exec(Irccdctl &client, const std::vector<std::string> &args) override;
};

} // !cli

} // !irccd

#endif // !IRCCD_CLI_HPP
