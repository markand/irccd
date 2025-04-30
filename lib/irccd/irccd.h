/*
 * irccd.h -- main irccd object
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_H
#define IRCCD_H

#include <stddef.h>

struct ev_loop;

struct irc_event;
struct irc_hook;
struct irc_plugin;
struct irc_plugin_loader;
struct irc_rule;
struct irc_server;

#if defined(__cplusplus)
extern "C" {
#endif

struct irccd {
	struct irc_server *servers;
	struct irc_plugin *plugins;
	struct irc_plugin_loader *plugin_loaders;
	struct irc_rule *rules;
	struct irc_hook *hooks;
};

/**
 * Read-only access to the public types.
 */
extern const struct irccd *irccd;

/**
 * Initialize the bot with the event loop.
 *
 * \param loop the libev event loop to pass (maybe NULL)
 */
void
irc_bot_init(struct ev_loop *loop);

/**
 * Return the event loop associated with the bot.
 *
 * \return the event loop
 */
struct ev_loop *
irc_bot_loop(void);

/**
 * Add a new server to the bot.
 *
 * The bot will start its connection so user should not do it by itself even
 * though this is not considered an issue. However, if the server already exists
 * the server should be deleted by the caller.
 *
 * \pre s != NULL
 * \param s the server to add
 * \return 0 on success or -1 on error
 */
int
irc_bot_server_add(struct irc_server *s);

/**
 * Find a server by its name.
 *
 * \pre name != NULL
 * \param name the server name to find
 * \return the server or NULL on error
 */
struct irc_server *
irc_bot_server_get(const char *name);

/**
 * Remove a server by name.
 *
 * \pre name != NULL
 * \param name the server name to remove
 */
void
irc_bot_server_remove(const char *);

/**
 * Remove all servers from the bot and disconnect them.
 */
void
irc_bot_server_clear(void);

/**
 * Register a new plugin into the bot.
 *
 * The user should not invoke the "load" callback by itself because the bot will
 * do it automatically.
 */
int
irc_bot_plugin_add(struct irc_plugin *plg);

/**
 * Search a plugin from the filesystem and return it for convenience.
 *
 * The plugin will only by searched and returned and not automatically loaded
 * into the bot itself. This is designed to allow user customization prior to
 * installation.
 *
 * If the path is NULL, irccd will search the plugin through the registered
 * plugin loader interfaces and default paths.
 *
 * \pre name != NULL
 * \param name the plugin name
 * \param path path to the plugin (may be NULL)
 * \return the plugin if found or NULL on error
 */
struct irc_plugin *
irc_bot_plugin_search(const char *name, const char *path);

/**
 * Get a plugin by name.
 *
 * \pre name != NULL
 * \param name the plugin name
 * \return the plugin or NULL if not found
 */
struct irc_plugin *
irc_bot_plugin_get(const char *name);

/**
 * Remove a plugin specified by name.
 *
 * If the plugin is found, the function "unload" will also be invoked first.
 *
 * \pre name != NULL
 * \param name the plugin name to remove
 */
void
irc_bot_plugin_remove(const char *name);

/**
 * Remove all plugins from the bot.
 */
void
irc_bot_plugin_clear(void);

/**
 * Register a new plugin loader into the bot.
 *
 * The bot does not take ownership of loader and must exist until the bot is
 * destroyed.
 *
 * \pre loader != NULL
 * \param loader the loader to append
 */
void
irc_bot_plugin_loader_add(struct irc_plugin_loader *loader);

/**
 * Register a new rule at the given index.
 *
 * The bot takes ownership of the rule.
 *
 * If the position is greater than the number of rules, it is appended to the
 * end.
 *
 * \pre rule != NULL
 * \param rule the rule to append
 * \param pos the position
 */
void
irc_bot_rule_insert(struct irc_rule *rule, size_t pos);

/**
 * Find a rule at the given index.
 *
 * \pre pos < irc_bot_rule_size()
 * \param pos the rule position
 * \return the rule
 */
struct irc_rule *
irc_bot_rule_get(size_t pos);

/**
 * Move a rule.
 *
 * \param from the original position
 * \param to the destination position
 */
void
irc_bot_rule_move(size_t from, size_t to);

/**
 * Remove a rule at the given position.
 *
 * \pre pos < irc_bot_rule_size()
 * \param pos the rule position
 */
void
irc_bot_rule_remove(size_t pos);

/**
 * Return the number of rules active in the bot.
 *
 * \return the number of rules
 */
size_t
irc_bot_rule_size(void);

/**
 * Remove all rules from the bot.
 */
void
irc_bot_rule_clear(void);

/**
 * Add a new rule into the bot.
 *
 * The bot takes ownership of the hook.
 *
 * \pre hook != NULL
 * \param hook the hook to add
 * \return 0 on success or -1 on error
 */
int
irc_bot_hook_add(struct irc_hook *hook);

/**
 * Find a hook by name.
 *
 * \pre name != NULL
 * \param name the hook name
 * \return the hook or NULL if not found
 */
struct irc_hook *
irc_bot_hook_get(const char *name);

/**
 * Remove a hook by name.
 *
 * \pre name != NULL
 * \param name the hook name to remove
 */
void
irc_bot_hook_remove(const char *name);

/**
 * Remove all hooks from the bot.
 */
void
irc_bot_hook_clear(void);

/**
 * Dispatch an IRC event to all plugins and hooks.
 *
 * \pre ev != NULL
 * \param ev the event to dispatch
 */
void
irc_bot_dispatch(const struct irc_event *ev);

/**
 * Stop the event loop and destroy every resource associated with the bot.
 */
void
irc_bot_finish(void);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_H */
