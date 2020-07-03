/*
 * cli.hpp -- command line for irccdctl
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_CTL_CLI_HPP
#define IRCCD_CTL_CLI_HPP

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <json.hpp>

namespace irccd::ctl {

class controller;

// {{{ cli

/**
 * \brief Abstract CLI class.
 */
class cli {
public:
	/**
	 * \brief Convenient handler for request function.
	 */
	using handler_t = std::function<void (nlohmann::json)>;

	/**
	 * \brief Command constructor factory.
	 */
	using constructor = std::function<std::unique_ptr<cli> ()>;

	/**
	 * \brief Registry of all commands.
	 */
	static const std::vector<constructor> registry;

private:
	void recv_response(ctl::controller&, nlohmann::json, handler_t);

protected:
	/**
	 * Convenient request helper.
	 *
	 * This function send and receive the response for the given request. It
	 * checks for an error code or string in the command result and throws it if
	 * any.
	 *
	 * If handler is not null, it will be called once the command result has
	 * been received.
	 *
	 * This function may executes successive read calls until we get the
	 * response.
	 *
	 * \param ctl the controller
	 * \param json the json object
	 * \param handler the optional handler
	 */
	void request(ctl::controller& ctl, nlohmann::json json, handler_t handler = nullptr);

public:
	/**
	 * Default constructor.
	 */
	cli() noexcept = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~cli() noexcept = default;

	/**
	 * Return the command name.
	 *
	 * \return the name
	 */
	virtual auto get_name() const noexcept -> std::string_view = 0;

	/**
	 * Execute the command.
	 *
	 * \param ctl the controller
	 * \param args the user arguments
	 */
	virtual void exec(ctl::controller& ctl, const std::vector<std::string>& args) = 0;
};

// }}}

// {{{ hook_add_cli

/**
 * \brief Implementation of irccdctl hook-add.
 */
class hook_add_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ hook_list_cli

/**
 * \brief Implementation of irccdctl hook-list.
 */
class hook_list_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ hook_remove_cli

/**
 * \brief Implementation of irccdctl hook-remove.
 */
class hook_remove_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ plugin_config_cli

/**
 * \brief Implementation of irccdctl plugin-config.
 */
class plugin_config_cli : public cli {
private:
	void set(ctl::controller&, const std::vector<std::string>&);
	void get(ctl::controller&, const std::vector<std::string>&);
	void getall(ctl::controller&, const std::vector<std::string>&);

public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ plugin_info_cli

/**
 * \brief Implementation of irccdctl plugin-info.
 */
class plugin_info_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ plugin_list_cli

/**
 * \brief Implementation of irccdctl plugin-list.
 */
class plugin_list_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ plugin_load_cli

/**
 * \brief Implementation of irccdctl plugin-load.
 */
class plugin_load_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ plugin_reload_cli

/**
 * \brief Implementation of irccdctl plugin-reload.
 */
class plugin_reload_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ plugin_unload_cli

/**
 * \brief Implementation of irccdctl plugin-unload.
 */
class plugin_unload_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ rule_add_cli

/**
 * \brief Implementation of irccdctl rule-add.
 */
class rule_add_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ rule_edit_cli

/**
 * \brief Implementation of irccdctl rule-edit.
 */
class rule_edit_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ rule_info_cli

/**
 * \brief Implementation of irccdctl rule-info.
 */
class rule_info_cli : public cli {
public:
	/**
	 * Pretty print a rule to stdout.
	 *
	 * \param json the rule information
	 * \param index the rule index
	 */
	static void print(const nlohmann::json& json, int index = 0);

	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ rule_list_cli

/**
 * \brief Implementation of irccdctl rule-list.
 */
class rule_list_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ rule_move_cli

/**
 * \brief Implementation of irccdctl rule-move.
 */
class rule_move_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ rule_remove_cli

/**
 * \brief Implementation of irccdctl rule-remove.
 */
class rule_remove_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_connect_cli

/**
 * \brief Implementation of irccdctl server-connect.
 */
class server_connect_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_disconnect_cli

/**
 * \brief Implementation of irccdctl server-disconnect.
 */
class server_disconnect_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_info_cli

/**
 * \brief Implementation of irccdctl server-info.
 */
class server_info_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_invite_cli

/**
 * \brief Implementation of irccdctl server-invite.
 */
class server_invite_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_join_cli

/**
 * \brief Implementation of irccdctl server-join.
 */
class server_join_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_kick_cli

/**
 * \brief Implementation of irccdctl server-kick.
 */
class server_kick_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_list_cli

/**
 * \brief Implementation of irccdctl server-list.
 */
class server_list_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_me_cli

/**
 * \brief Implementation of irccdctl server-me.
 */
class server_me_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_message_cli

/**
 * \brief Implementation of irccdctl server-message.
 */
class server_message_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_mode_cli

/**
 * \brief Implementation of irccdctl server-mode.
 */
class server_mode_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_nick_cli

/**
 * \brief Implementation of irccdctl server-nick.
 */
class server_nick_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_notice_cli

/**
 * \brief Implementation of irccdctl server-notice.
 */
class server_notice_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_part_cli

/**
 * \brief Implementation of irccdctl server-part.
 */
class server_part_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_reconnect_cli

/**
 * \brief Implementation of irccdctl server-reconnect.
 */
class server_reconnect_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ server_topic_cli

/**
 * \brief Implementation of irccdctl server-topic.
 */
class server_topic_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

// {{{ watch_cli

/**
 * \brief Implementation of irccdctl watch.
 */
class watch_cli : public cli {
public:
	/**
	 * \copydoc cli::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc cli::exec
	 */
	void exec(ctl::controller& irccdctl, const std::vector<std::string>& args) override;
};

// }}}

} // !irccd::ctl

#endif // !IRCCD_CTL_CLI_HPP
