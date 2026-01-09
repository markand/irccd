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

#include <sys/types.h>

#include <ev.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct irc_event;

/**
 * \brief Hook active children.
 */
struct irc_hook_child {
	/**
	 * (read-only)
	 *
	 * Process PID.
	 */
	pid_t pid;

	/**
	 * (read-only)
	 *
	 * Process watcher for reaping.
	 */
	struct ev_child child;

	/**
	 * (read-only)
	 *
	 * Timer in case the process would not stop.
	 */
	struct ev_timer timer;

	/**
	 * (read-only, borrowed)
	 *
	 * Parent hook in which this process belong to.
	 */
	struct irc_hook *parent;

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Next child in the linked list.
	 */
	struct irc_hook_child *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

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

	/**
	 * (private)
	 *
	 * Next hook in the linked list.
	 */
	struct irc_hook *next;

	/**
	 * (private)
	 *
	 * Linked list of active processes.
	 */
	struct irc_hook_child *children;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * Create a new hook.
 *
 * \pre name != NULL
 * \pre path != NULL
 * \param name the hook name
 * \param path path to the hook script/executable
 * \return a new hook
 */
struct irc_hook *
irc_hook_new(const char *name, const char *path);

/**
 * Invoke the hook by spawning a new child with the given event.
 *
 * The child process runs independently and will be reaped automatically when it
 * exits.
 *
 * \pre hook != NULL
 * \pre ev != NULL
 * \param hook the hook
 * \param ev the event to pass to the hook child
 */
void
irc_hook_invoke(struct irc_hook *hook, const struct irc_event *ev);

/**
 * Destroy the hook and possibly the active children if any.
 *
 * This function blocks if there is one or more active children, calling
 * `ev_run` until they are entirely destroyed.
 *
 * The hook receive a SIGTERM signal first and if they would not exit after a
 * short amount of time, SIGKILL is then raise.
 */
void
irc_hook_free(struct irc_hook *hook);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_HOOK_H */
