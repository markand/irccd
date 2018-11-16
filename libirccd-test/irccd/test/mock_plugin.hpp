/*
 * mock_plugin.hpp -- mock plugin
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

#ifndef IRCCD_TEST_MOCK_PLUGIN_HPP
#define IRCCD_TEST_MOCK_PLUGIN_HPP

/**
 * \file mock_plugin.hpp
 * \brief Mock plugin.
 */

#include <irccd/daemon/plugin.hpp>

#include "mock.hpp"

namespace irccd::test {

/**
 * \brief Mock plugin.
 */
class mock_plugin : public daemon::plugin, public mock {
private:
	map options_;
	map formats_;
	map paths_;

public:
	using plugin::plugin;

	/**
	 * \copydoc daemon::plugin::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc daemon::plugin::get_author
	 */
	auto get_author() const noexcept -> std::string_view override;

	/**
	 * \copydoc daemon::plugin::get_license
	 */
	auto get_license() const noexcept -> std::string_view override;

	/**
	 * \copydoc daemon::plugin::get_summary
	 */
	auto get_summary() const noexcept -> std::string_view override;

	/**
	 * \copydoc daemon::plugin::get_version
	 */
	auto get_version() const noexcept -> std::string_view override;

	/**
	 * \copydoc daemon::plugin::get_options
	 */
	auto get_options() const -> map override;

	/**
	 * \copydoc daemon::plugin::set_options
	 */
	void set_options(const map& map) override;

	/**
	 * \copydoc daemon::plugin::get_formats
	 */
	auto get_formats() const -> map override;

	/**
	 * \copydoc daemon::plugin::set_formats
	 */
	void set_formats(const map& map) override;

	/**
	 * \copydoc daemon::plugin::get_paths
	 */
	auto get_paths() const -> map override;

	/**
	 * \copydoc daemon::plugin::set_paths
	 */
	void set_paths(const map& map) override;

	/**
	 * \copydoc daemon::plugin::handle_command
	 */
	void handle_command(daemon::bot& bot, const daemon::message_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_connect
	 */
	void handle_connect(daemon::bot& bot, const daemon::connect_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_disconnect
	 */
	void handle_disconnect(daemon::bot& bot, const daemon::disconnect_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_invite
	 */
	void handle_invite(daemon::bot& bot, const daemon::invite_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_join
	 */
	void handle_join(daemon::bot& bot, const daemon::join_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_kick
	 */
	void handle_kick(daemon::bot& bot, const daemon::kick_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_load
	 */
	void handle_load(daemon::bot& bot) override;

	/**
	 * \copydoc daemon::plugin::handle_message
	 */
	void handle_message(daemon::bot& bot, const daemon::message_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_me
	 */
	void handle_me(daemon::bot& bot, const daemon::me_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_mode
	 */
	void handle_mode(daemon::bot& bot, const daemon::mode_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_names
	 */
	void handle_names(daemon::bot& bot, const daemon::names_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_nick
	 */
	void handle_nick(daemon::bot& bot, const daemon::nick_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_notice
	 */
	void handle_notice(daemon::bot& bot, const daemon::notice_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_part
	 */
	void handle_part(daemon::bot& bot, const daemon::part_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_reload
	 */
	void handle_reload(daemon::bot& bot) override;

	/**
	 * \copydoc daemon::plugin::handle_topic
	 */
	void handle_topic(daemon::bot& bot, const daemon::topic_event& event) override;

	/**
	 * \copydoc daemon::plugin::handle_unload
	 */
	void handle_unload(daemon::bot& bot) override;

	/**
	 * \copydoc daemon::plugin::handle_whois
	 */
	void handle_whois(daemon::bot& bot, const daemon::whois_event& event) override;
};

} // !irccd::test

#endif // !IRCCD_TEST_MOCK_PLUGIN_HPP
