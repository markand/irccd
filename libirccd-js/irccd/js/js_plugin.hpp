/*
 * js_plugin.hpp -- JavaScript plugins for irccd
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

#ifndef IRCCD_JS_PLUGIN_HPP
#define IRCCD_JS_PLUGIN_HPP

/**
 * \file js_plugin.hpp
 * \brief JavaScript plugins for irccd.
 */

#include <irccd/daemon/plugin.hpp>
#include <irccd/daemon/server.hpp>

#include "duk.hpp"

/**
 * \brief Javascript namespace
 */
namespace irccd::js {

class js_api;

/**
 * \ingroup plugins
 * \brief JavaScript plugins for irccd.
 */
class js_plugin : public plugin {
public:
	/**
	 * Global property where to read/write plugin configuration (object).
	 */
	static inline const std::string_view config_property{"\xff\xff""config"};

	/**
	 * Global property where to read/write plugin formats (object).
	 */
	static inline const std::string_view format_property{"\xff\xff""formats"};

	/**
	 * Global property where paths are defined (object).
	 */
	static inline const std::string_view paths_property{"\xff\xff""paths"};

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
	js_plugin(std::string id, std::string path);

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

/**
 * \ingroup plugins
 * \brief Implementation for searching Javascript plugins.
 */
class js_plugin_loader : public plugin_loader {
public:
	/**
	 * \brief The list of Javascript API modules.
	 */
	using modules = std::vector<std::unique_ptr<js_api>>;

private:
	irccd& irccd_;
	modules modules_;

public:
	/**
	 * Constructor.
	 *
	 * \param irccd the irccd instance
	 * \param directories directories to search
	 * \param extensions extensions to search
	 */
	js_plugin_loader(irccd& irccd,
	                 std::vector<std::string> directories = {},
	                 std::vector<std::string> extensions = {".js"}) noexcept;

	/**
	 * Destructor defaulted.
	 */
	~js_plugin_loader() noexcept;

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
	 * \copydoc plugin_loader::open
	 */
	auto open(std::string_view id, std::string_view file) -> std::shared_ptr<plugin>;
};

namespace duk {

/**
 * \brief Specialization for type_traits<whois_info>
 */
template <>
struct type_traits<whois_info> : public std::true_type {
	/**
	 * Push a whois_info.
	 *
	 * \param ctx the Duktape context
	 * \param who the information
	 */
	static void push(duk_context* ctx, const whois_info& who);
};

} // !duk

} // !irccd::js

#endif // !IRCCD_PLUGIN_JS_HPP
