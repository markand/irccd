/*
 * command.hpp -- remote command
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_COMMAND_HPP
#define IRCCD_DAEMON_COMMAND_HPP

/**
 * \file command.hpp
 * \brief Remote commands.
 */

#include <irccd/sysconfig.hpp>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include <irccd/json_util.hpp>

namespace irccd::daemon {

class bot;
class transport_client;

// {{{ command

/**
 * \brief Server side remote command
 * \ingroup transports
 */
class command {
public:
	/**
	 * \brief Convenient alias.
	 */
	using document = json_util::deserializer;

	/**
	 * \brief Command constructor factory.
	 */
	using constructor = std::function<std::unique_ptr<command> ()>;

	/**
	 * \brief Registry of all commands.
	 */
	static auto registry() noexcept -> const std::vector<constructor>&;

	/**
	 * Default destructor virtual.
	 */
	virtual ~command() = default;

	/**
	 * Return the command name, must not have spaces.
	 *
	 * \return the command name
	 */
	virtual auto get_name() const noexcept -> std::string_view = 0;

	/**
	 * Execute the command.
	 *
	 * If the command throw an exception, the error is sent to the client so be
	 * careful about sensitive information.
	 *
	 * The implementation should use client.success() or client.error() to send
	 * some data.
	 *
	 * \param bot the irccd instance
	 * \param client the client
	 * \param args the client arguments
	 */
	virtual void exec(bot& bot, transport_client& client, const document& args) = 0;
};

// }}}

// {{{ plugin_config_command

/**
 * \brief Implementation of plugin-config transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - plugin_error::not_found
 */
class plugin_config_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ plugin_info_command

/**
 * \brief Implementation of plugin-info transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - plugin_error::not_found
 */
class plugin_info_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ plugin_list_command

/**
 * \brief Implementation of plugin-list transport command.
 * \ingroup transports
 */
class plugin_list_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ plugin_load_command

/**
 * \brief Implementation of plugin-load transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - plugin_error::already_exists
 * - plugin_error::not_found
 * - plugin_error::exec_error
 */
class plugin_load_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ plugin_reload_command

/**
 * \brief Implementation of plugin-reload transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - plugin_error::not_found
 * - plugin_error::exec_error
 */
class plugin_reload_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ plugin_unload_command

/**
 * \brief Implementation of plugin-unload transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - plugin_error::not_found
 * - plugin_error::exec_error
 */
class plugin_unload_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ rule_add_command

/**
 * \brief Implementation of rule-add transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - rule_error::invalid_action
 */
class rule_add_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ rule_edit_command

/**
 * \brief Implementation of rule-edit transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - rule_error::invalid_index
 * - rule_error::invalid_action
 */
class rule_edit_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ rule_info_command

/**
 * \brief Implementation of rule-info transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - rule_error::invalid_index
 */
class rule_info_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ rule_list_command

/**
 * \brief Implementation of rule-list transport command.
 * \ingroup transports
 */
class rule_list_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ rule_move_command

/**
 * \brief Implementation of rule-move transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - rule_error::invalid_index
 */
class rule_move_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ rule_remove_command

/**
 * \brief Implementation of rule-remove transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - rule_error::invalid_index
 */
class rule_remove_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_connect_command

/**
 * \brief Implementation of server-connect transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::already_exists,
 * - server_error::invalid_hostname,
 * - server_error::invalid_identifier,
 * - server_error::invalid_port_number,
 * - server_error::ssl_disabled.
 */
class server_connect_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_disconnect_command

/**
 * \brief Implementation of server-disconnect transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_disconnect_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_info_command

/**
 * \brief Implementation of server-info transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_info_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_invite_command

/**
 * \brief Implementation of server-invite transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::invalid_nickname,
 * - server_error::not_found.
 */
class server_invite_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_join_command

/**
 * \brief Implementation of server-join transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_join_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_kick_command

/**
 * \brief Implementation of server-kick transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::invalid_nickname,
 * - server_error::not_found.
 */
class server_kick_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_list_command

/**
 * \brief Implementation of server-list transport command.
 * \ingroup transports
 */
class server_list_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_me_command

/**
 * \brief Implementation of server-me transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_me_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_message_command

/**
 * \brief Implementation of server-message transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_message_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_mode_command

/**
 * \brief Implementation of server-mode transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::invalid_mode,
 * - server_error::not_found.
 */
class server_mode_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_nick_command

/**
 * \brief Implementation of server-nick transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_identifier,
 * - server_error::invalid_nickname,
 * - server_error::not_found.
 */
class server_nick_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_notice_command

/**
 * \brief Implementation of server-notice transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_notice_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_part_command

/**
 * \brief Implementation of server-part transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_part_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_reconnect_command

/**
 * \brief Implementation of server-reconnect transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_reconnect_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

// {{{ server_topic_command

/**
 * \brief Implementation of server-topic transport command.
 * \ingroup transports
 *
 * Replies:
 *
 * - server_error::invalid_channel,
 * - server_error::invalid_identifier,
 * - server_error::not_found.
 */
class server_topic_command : public command {
public:
	/**
	 * \copydoc command::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc command::exec
	 */
	void exec(bot& bot, transport_client& client, const document& args) override;
};

// }}}

} // !irccd::daemon

#endif // !IRCCD_DAEMON_COMMAND_HPP
