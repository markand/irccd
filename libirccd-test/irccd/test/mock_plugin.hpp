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
class mock_plugin : public plugin, public mock {
private:
	map options_;
	map formats_;
	map paths_;

public:
	using plugin::plugin;

	/**
	 * \copydoc plugin::get_name
	 */
	auto get_name() const noexcept -> std::string_view override;

	/**
	 * \copydoc plugin::get_author
	 */
	auto get_author() const noexcept -> std::string_view override;

	/**
	 * \copydoc plugin::get_license
	 */
	auto get_license() const noexcept -> std::string_view override;

	/**
	 * \copydoc plugin::get_summary
	 */
	auto get_summary() const noexcept -> std::string_view override;

	/**
	 * \copydoc plugin::get_version
	 */
	auto get_version() const noexcept -> std::string_view override;

	/**
	 * \copydoc plugin::get_options
	 */
	auto get_options() const -> map override;

	/**
	 * \copydoc plugin::set_options
	 */
	void set_options(const map& map) override;

	/**
	 * \copydoc plugin::get_formats
	 */
	auto get_formats() const -> map override;

	/**
	 * \copydoc plugin::set_formats
	 */
	void set_formats(const map& map) override;

	/**
	 * \copydoc plugin::get_paths
	 */
	auto get_paths() const -> map override;

	/**
	 * \copydoc plugin::set_paths
	 */
	void set_paths(const map& map) override;

	/**
	 * \copydoc plugin::handle_command
	 */
	void handle_command(irccd& irccd, const message_event& event) override;

	/**
	 * \copydoc plugin::handle_connect
	 */
	void handle_connect(irccd& irccd, const connect_event& event) override;

	/**
	 * \copydoc plugin::handle_disconnect
	 */
	void handle_disconnect(irccd& irccd, const disconnect_event& event) override;

	/**
	 * \copydoc plugin::handle_invite
	 */
	void handle_invite(irccd& irccd, const invite_event& event) override;

	/**
	 * \copydoc plugin::handle_join
	 */
	void handle_join(irccd& irccd, const join_event& event) override;

	/**
	 * \copydoc plugin::handle_kick
	 */
	void handle_kick(irccd& irccd, const kick_event& event) override;

	/**
	 * \copydoc plugin::handle_load
	 */
	void handle_load(irccd& irccd) override;

	/**
	 * \copydoc plugin::handle_message
	 */
	void handle_message(irccd& irccd, const message_event& event) override;

	/**
	 * \copydoc plugin::handle_me
	 */
	void handle_me(irccd& irccd, const me_event& event) override;

	/**
	 * \copydoc plugin::handle_mode
	 */
	void handle_mode(irccd& irccd, const mode_event& event) override;

	/**
	 * \copydoc plugin::handle_names
	 */
	void handle_names(irccd& irccd, const names_event& event) override;

	/**
	 * \copydoc plugin::handle_nick
	 */
	void handle_nick(irccd& irccd, const nick_event& event) override;

	/**
	 * \copydoc plugin::handle_notice
	 */
	void handle_notice(irccd& irccd, const notice_event& event) override;

	/**
	 * \copydoc plugin::handle_part
	 */
	void handle_part(irccd& irccd, const part_event& event) override;

	/**
	 * \copydoc plugin::handle_reload
	 */
	void handle_reload(irccd& irccd) override;

	/**
	 * \copydoc plugin::handle_topic
	 */
	void handle_topic(irccd& irccd, const topic_event& event) override;

	/**
	 * \copydoc plugin::handle_unload
	 */
	void handle_unload(irccd& irccd) override;

	/**
	 * \copydoc plugin::handle_whois
	 */
	void handle_whois(irccd& irccd, const whois_event& event) override;
};

} // !irccd::test

#endif // !IRCCD_TEST_MOCK_PLUGIN_HPP
