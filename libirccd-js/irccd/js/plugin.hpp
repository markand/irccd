/*
 * plugin.hpp -- JavaScript plugins for irccd
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

#ifndef IRCCD_JS_PLUGIN_HPP
#define IRCCD_JS_PLUGIN_HPP

/**
 * \file plugin.hpp
 * \brief JavaScript plugins for irccd.
 */

#include <irccd/daemon/plugin.hpp>
#include <irccd/daemon/server.hpp>

#include "duk.hpp"

/**
 * \brief Javascript namespace
 */
namespace irccd::js {

class api;

/**
 * \ingroup js
 * \ingroup daemon-plugins
 * \brief JavaScript plugins for irccd.
 */
class plugin : public daemon::plugin {
public:
	/**
	 * Global property where to read/write plugin configuration (object).
	 */
	static inline const std::string_view config_property{DUK_HIDDEN_SYMBOL("config")};

	/**
	 * Global property where to read/write plugin templates (object).
	 */
	static inline const std::string_view templates_property{DUK_HIDDEN_SYMBOL("templates")};

	/**
	 * Global property where paths are defined (object).
	 */
	static inline const std::string_view paths_property{DUK_HIDDEN_SYMBOL("paths")};

private:
	// JavaScript context.
	mutable duk::context context_;

	// Path to Javascript script file.
	std::string path_;

	void push() noexcept;

	template <typename Value, typename... Args>
	void push(Value&& value, Args&&... args);

	template <typename... Args>
	void call(const std::string&, Args&&... args);

public:
	/**
	 * Constructor.
	 *
	 * \param id the plugin id
	 * \param path the path to the plugin
	 */
	plugin(std::string id, std::string path);

	/**
	 * Access the Duktape context.
	 *
	 * \return the context
	 */
	auto get_context() noexcept -> duk::context&;

	/**
	 * Open the script file associated.
	 */
	void open();

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
	 * \copydoc daemon::plugin::get_templates
	 */
	auto get_templates() const -> map override;

	/**
	 * \copydoc daemon::plugin::set_templates
	 */
	void set_templates(const map& map) override;

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

/**
 * \ingroup plugins
 * \brief Implementation for searching Javascript plugins.
 */
class plugin_loader : public daemon::plugin_loader {
public:
	/**
	 * \brief The list of Javascript API modules.
	 */
	using modules = std::vector<std::unique_ptr<api>>;

private:
	daemon::bot& bot_;
	modules modules_;

public:
	/**
	 * Constructor.
	 *
	 * \param bot the irccd instance
	 * \param directories directories to search
	 * \param extensions extensions to search
	 */
	plugin_loader(daemon::bot& bot,
	              std::vector<std::string> directories = {},
	              std::vector<std::string> extensions = {".js"}) noexcept;

	/**
	 * Destructor defaulted.
	 */
	~plugin_loader() noexcept;

	/**
	 * Get the list of modules.
	 *
	 * \return the modules
	 */
	auto get_modules() const noexcept -> const modules&;

	/**
	 * Overloaded function.
	 *
	 * \return the modules
	 */
	auto get_modules() noexcept -> modules&;

	/**
	 * \copydoc daemon::plugin_loader::open
	 */
	auto open(std::string_view id, std::string_view file) -> std::shared_ptr<daemon::plugin>;
};

namespace duk {

/**
 * \brief Specialization for type_traits<whois_info>
 */
template <>
struct type_traits<daemon::whois_info> : public std::true_type {
	/**
	 * Push a whois_info.
	 *
	 * \param ctx the Duktape context
	 * \param who the information
	 */
	static void push(duk_context* ctx, const daemon::whois_info& who);
};

} // !duk

} // !irccd::js

#endif // !IRCCD_PLUGIN_JS_HPP
