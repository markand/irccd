/*
 * hook_service.hpp -- irccd hook service
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

#ifndef IRCCD_DAEMON_HOOK_SERVICE_HPP
#define IRCCD_DAEMON_HOOK_SERVICE_HPP

/**
 * \file irccd/daemon/hook_service.hpp
 * \brief Irccd hook service.
 */

#include <functional>
#include <vector>
#include <utility>

#include "bot.hpp"
#include "hook.hpp"
#include "logger.hpp"

namespace irccd::daemon {

class bot;

/**
 * \brief Irccd hook service.
 */
class hook_service {
public:
	/**
	 * List of hooks.
	 */
	using hooks = std::vector<hook>;

private:
	bot& bot_;
	hooks hooks_;

public:
	/**
	 * Constructor.
	 *
	 * \param bot the bot
	 */
	hook_service(bot& bot) noexcept;

	/**
	 * Tells if a hook already exists.
	 *
	 * \param hook the hook to check
	 * \return true if hook is already present
	 */
	auto has(const hook& hook) const noexcept -> bool;

	/**
	 * Add a new hook.
	 *
	 * \param hook the hook
	 * \throw hook_error if the hook is already present
	 */
	void add(hook hook);

	/**
	 * Remove the specified hook.
	 *
	 * \param hook the hook to remove
	 */
	void remove(const hook& hook) noexcept;

	/**
	 * Get the list of hooks.
	 *
	 * \return the hooks
	 */
	auto list() const noexcept -> const hooks&;

	/**
	 * Overloaded function.
	 *
	 * \return the hooks
	 */
	auto list() noexcept -> hooks&;

	/**
	 * Remove all hooks.
	 */
	void clear() noexcept;

	/**
	 * Convenient function to call a hook member function for all hook
	 * present in the list.
	 *
	 * \param func the function to call (e.g. hook::handle_connect)
	 * \param args the arguments to the hook function
	 * \throw hook_error on errors
	 */
	template <typename Func, typename... Args>
	void dispatch(Func&& func, Args&&... args);
};

template <typename Func, typename... Args>
void hook_service::dispatch(Func&& func, Args&&... args)
{
	using std::invoke;
	using std::forward;
	using std::exception;

	for (auto& hook : hooks_) {
		// Protect to avoid stopping all next hooks.
		try {
			invoke(forward<Func>(func), hook, bot_, forward<Args>(args)...);
		} catch (const exception& ex) {
			bot_.get_log().warning(hook) << ex.what() << std::endl;
		}
	}
}

} // !irccd::daemon

#endif // !IRCCD_DAEMON_HOOK_SERVICE_HPP
