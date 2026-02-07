/*
 * hook.h -- irccd hooks
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_HOOK_H
#define IRCCD_HOOK_H

/**
 * \file hook.h
 * \brief Irccd hooks.
 *
 * Hooks are lightweight alternatives to plugins and are launched upon IRC
 * events but are more limited.
 */

#include <ev.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct irc_event;

/**
 * \brief IRC event hook.
 */
struct irc_hook {
	/**
	 * (read-write)
	 *
	 * Hook name.
	 */
	char *name;

	/**
	 * (read-write)
	 *
	 * Path to the hook to be executed.
	 */
	char *path;

	/**
	 * \cond IRC_PRIVATE
	 */

	struct irc_hook *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * Create a new hook.
 *
 * \param name the hook name
 * \param path path to the hook script/executable
 * \return a new hook
 */
struct irc_hook *
irc_hook_new(const char *name, const char *path);

/**
 * Invoke the hook and wait for its termination.
 *
 * \note This function will block until the hook terminated.
 * \param ev the event to pass to the hook child process
 */
void
irc_hook_invoke(struct irc_hook *hook, const struct irc_event *ev);

/**
 * Destroy the hook.
 */
void
irc_hook_free(struct irc_hook *hook);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_HOOK_H */
