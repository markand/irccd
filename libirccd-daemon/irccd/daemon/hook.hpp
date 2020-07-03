/*
 * hook.hpp -- irccd hooks
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

#ifndef IRCCD_DAEMON_HOOK_HPP
#define IRCCD_DAEMON_HOOK_HPP

/**
 * \file irccd/daemon/hook.hpp
 * \brief irccd hooks
 */

#include <irccd/sysconfig.hpp>

#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace irccd::daemon {

class bot;

struct connect_event;
struct disconnect_event;
struct invite_event;
struct join_event;
struct kick_event;
struct me_event;
struct message_event;
struct mode_event;
struct nick_event;
struct notice_event;
struct part_event;
struct topic_event;

/**
 * \brief Event hook.
 *
 * A hook is a lightweight alternative to plugins, it is executed once an event
 * arrive and can be written in any language.
 */
class hook {
private:
	std::string id_;
	std::string path_;

public:
	/**
	 * Construct a hook.
	 *
	 * This does not check the presence of the script.
	 *
	 * \pre id must be a valid identifier
	 * \pre path must not be empty
	 * \param id the hook id
	 * \param path the path to the hook
	 */
	hook(std::string id, std::string path) noexcept;

	/**
	 * Get user unique id.
	 *
	 * \return the hook id
	 */
	auto get_id() const noexcept -> const std::string&;

	/**
	 * Get path to the hook.
	 *
	 * \return the path
	 */
	auto get_path() const noexcept -> const std::string&;

	/**
	 * Similar interface to plugin::handle_connect.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_connect(bot& bot, const connect_event& event);

	/**
	 * Similar interface to plugin::handle_disconnect.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_disconnect(bot& bot, const disconnect_event& event);

	/**
	 * Similar interface to plugin::handle_invite.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_invite(bot& bot, const invite_event& event);

	/**
	 * Similar interface to plugin::handle_join.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_join(bot& bot, const join_event& event);

	/**
	 * Similar interface to plugin::handle_kick.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_kick(bot& bot, const kick_event& event);

	/**
	 * Similar interface to plugin::handle_message.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_message(bot& bot, const message_event& event);

	/**
	 * Similar interface to plugin::handle_me.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_me(bot& bot, const me_event& event);

	/**
	 * Similar interface to plugin::handle_mode.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_mode(bot& bot, const mode_event& event);

	/**
	 * Similar interface to plugin::handle_nick.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_nick(bot& bot, const nick_event& event);

	/**
	 * Similar interface to plugin::handle_notice.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_notice(bot& bot, const notice_event& event);

	/**
	 * Similar interface to plugin::handle_part.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_part(bot& bot, const part_event& event);

	/**
	 * Similar interface to plugin::handle_topic.
	 *
	 * \param bot the irccd instance
	 * \param event the event
	 */
	void handle_topic(bot& bot, const topic_event& event);
};

/**
 * Equality operator.
 *
 * \param lhs the left side
 * \param rhs the right side
 * \return true if they equals
 */
auto operator==(const hook& lhs, const hook& rhs) noexcept -> bool;

/**
 * Equality operator.
 *
 * \param lhs the left side
 * \param rhs the right side
 * \return false if they equals
 */
auto operator!=(const hook& lhs, const hook& rhs) noexcept -> bool;

/**
 * \brief Hook error.
 */
class hook_error : public std::system_error {
public:
	/**
	 * \brief Plugin related errors.
	 */
	enum error {
		//!< No error.
		no_error = 0,

		//!< The specified identifier is invalid.
		invalid_identifier,

		//!< The specified hook is not found.
		not_found,

		//!< Invalid path given.
		invalid_path,

		//!< The hook was unable to run the function.
		exec_error,

		//!< The hook is already loaded.
		already_exists,
	};

private:
	std::string id_;
	std::string message_;

public:
	/**
	 * Constructor.
	 *
	 * \param code the error code
	 * \param id the hook id
	 * \param message the optional message (e.g. error from hook)
	 */
	hook_error(error code, std::string id, std::string message = "");

	/**
	 * Get the hook identifier.
	 *
	 * \return the id
	 */
	auto get_id() const noexcept -> const std::string&;

	/**
	 * Get the additional message.
	 *
	 * \return the message
	 */
	auto get_message() const noexcept -> const std::string&;

	/**
	 * Get message appropriate for use with logger.
	 *
	 * \return the error message
	 */
	auto what() const noexcept -> const char* override;
};

/**
 * Get the hook error category singleton.
 *
 * \return the singleton
 */
auto hook_category() -> const std::error_category&;

/**
 * Create a std::error_code from hook_error::error enum.
 *
 * \param e the error code
 * \return the error code
 */
auto make_error_code(hook_error::error e) -> std::error_code;

namespace logger {

template <typename T>
struct type_traits;

/**
 * \brief Specialization for hook.
 * \ingroup daemon-loggers-traits
 */
template <>
struct type_traits<hook> {
	/**
	 * Get 'hook' category.
	 *
	 * \param hook the hook
	 * \return hook
	 */
	static auto get_category(const hook& hook) -> std::string_view;

	/**
	 * Get the hook id.
	 *
	 * \param hook the hook
	 * \return the hook id
	 */
	static auto get_component(const hook& hook) -> std::string_view;
};

} // !logger

} // !irccd::daemon

/**
 * \cond IRCCD_HIDDEN_SYMBOLS
 */

namespace std {

template <>
struct is_error_code_enum<irccd::daemon::hook_error::error> : public std::true_type {
};

} // !std

/**
 * \endcond
 */

#endif // !IRCCD_DAEMON_PLUGIN_HPP
