/*
 * plugin.h -- abstract plugin
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

#ifndef IRCCD_PLUGIN_H
#define IRCCD_PLUGIN_H

/**
 * \file plugin.h
 * \brief Abstract plugin.
 *
 * This module handles the creation of the abstract plugin interface and loaders
 * to find them in a generic manner.
 */

#include "config.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct irc_event;

/**
 * \brief Default plugin license.
 */
#define IRC_PLUGIN_DEFAULT_LICENSE "ISC"

/**
 * \brief Default plugin version (set to irccd's version).
 */
#define IRC_PLUGIN_DEFAULT_VERSION IRCCD_VERSION

/**
 * \brief Default plugin author.
 */
#define IRC_PLUGIN_DEFAULT_AUTHOR "nobody"

/**
 * \brief Default plugin description.
 */
#define IRC_PLUGIN_DEFAULT_DESCRIPTION "no description"

/**
 * \brief Abstract plugin interface.
 *
 * Most of the plugin consists of internal callbacks to be defined by the plugin
 * loader after being initialized.
 */
struct irc_plugin {
	/**
	 * (read-only)
	 *
	 * Plugin name.
	 */
	char *name;

	/**
	 * (read-only)
	 *
	 * Plugin license.
	 */
	char *license;

	/**
	 * (read-only)
	 *
	 * Plugin version.
	 */
	char *version;

	/**
	 * (read-only)
	 *
	 * Plugin author.
	 */
	char *author;

	/**
	 * (read-only)
	 *
	 * Plugin human readable description.
	 */
	char *description;

	/**
	 * (read-write, optional)
	 *
	 * Set a plugin template.
	 *
	 * \param self this plugin
	 * \param key the template name
	 * \param value the new template value
	 */
	void (*set_template)(struct irc_plugin *self, const char *key, const char *value);

	/**
	 * (read-write, optional)
	 *
	 * Get a plugin template value.
	 *
	 * \param self this plugin
	 * \param key the template name
	 * \return the template value (borrowed)
	 */
	const char * (*get_template)(struct irc_plugin *self, const char *key);

	/**
	 * (read-write, optional)
	 *
	 * Get the list of templates this plugin supports in a NULL terminated
	 * list of strings.
	 *
	 * \param self this plugin
	 * \return a NULL terminated list of template keys (borrowed)
	 */
	const char * const * (*get_templates)(struct irc_plugin *self);

	/**
	 * (read-write, optional)
	 *
	 * Set a plugin path.
	 *
	 * \param self this plugin
	 * \param key the path name
	 * \param value the new path value
	 */
	void (*set_path)(struct irc_plugin *self, const char *key, const char *value);

	/**
	 * (read-write, optional)
	 *
	 * Get a plugin path value.
	 *
	 * \param self this plugin
	 * \param key the path name
	 * \return the path value (borrowed)
	 */
	const char * (*get_path)(struct irc_plugin *self, const char *key);

	/**
	 * (read-write, optional)
	 *
	 * Get the list of paths this plugin supports in a NULL terminated
	 * list of strings.
	 *
	 * \param self this plugin
	 * \return a NULL terminated list of path keys (borrowed)
	 */
	const char * const * (*get_paths)(struct irc_plugin *self);

	/**
	 * (read-write, optional)
	 *
	 * Set a plugin option.
	 *
	 * \param self this plugin
	 * \param key the option name
	 * \param value the new option value
	 */
	void (*set_option)(struct irc_plugin *self, const char *key, const char *value);

	/**
	 * (read-write, optional)
	 *
	 * Get a plugin option value.
	 *
	 * \param self this plugin
	 * \param key the option name
	 * \return the option value (borrowed)
	 */
	const char *(*get_option)(struct irc_plugin *self, const char *key);

	/**
	 * (read-write, optional)
	 *
	 * Get the list of options this plugin supports in a NULL terminated
	 * list of strings.
	 *
	 * \param self this plugin
	 * \return a NULL terminated list of option keys (borrowed)
	 */
	const char * const *(*get_options)(struct irc_plugin *self);

	/**
	 * (read-write, optional)
	 *
	 * Load the plugin.
	 *
	 * This function is called after the plugin has been initialized and is
	 * about to be registered into the bot instance.
	 *
	 * If the callback returns non-zero, the plugin is discarded.
	 *
	 * \param self this plugin
	 * \return 0 on success or -1 on error
	 */
	int (*load)(struct irc_plugin *self);

	/**
	 * (read-write, optional)
	 *
	 * Reload the plugin.
	 *
	 * This is a custom function that is simply available to perform some
	 * reloading steps.
	 *
	 * \param self this plugin
	 */
	void (*reload)(struct irc_plugin *self);

	/**
	 * (read-write, optional)
	 *
	 * Function being called just after it was removed from the irccd
	 * instance.
	 *
	 * \param self this plugin
	 */
	void (*unload)(struct irc_plugin *self);

	/**
	 * (read-write, optional)
	 *
	 * Function called when a new IRC event happened.
	 *
	 * \param self this plugin
	 * \param ev the IRC event
	 */
	void (*handle)(struct irc_plugin *self, const struct irc_event *ev);

	/**
	 * (read-write, optional)
	 *
	 * Function called when the plugin is destroyed.
	 *
	 * In this callback, user must not inspect any of the internal variables
	 * as they are already destructed. This is because the plugin itself may
	 * be free'd by the user callback.
	 *
	 * \param self this plugin
	 */
	void (*finish)(struct irc_plugin *self);

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Next plugin in the linked list.
	 */
	struct irc_plugin *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * \brief Abstract plugin loader interface.
 *
 * This structure is an helper to find plugins on the system.
 */
struct irc_plugin_loader {
	/**
	 * (read-only)
	 *
	 * Colon separated list of paths to directories where to search plugins.
	 */
	char *paths;

	/**
	 * (read-only)
	 *
	 * Colon separated list of suffixes for this plugin loader excluding '.'
	 *
	 * Examples:
	 *
	 * - `dylib:so`
	 * - `js`
	 */
	char *extensions;

	/**
	 * (read-write)
	 *
	 * Try to open the plugin at the given absolute path.
	 *
	 * The function should create a ::irc_plugin structure initialized with
	 * proper callbacks and returned as-is. It doesn't matter if the plugin
	 * returned is dynamically allocated or not but it must be valid until
	 * the bot removes it by itself.
	 *
	 * The function must not forget to initialize the plugin using
	 * ::irc_plugin_init and possibly ::irc_plugin_set_info before
	 * returning. It must also not invoke any load function which is already
	 * done when the bot tries to register the plugin.
	 *
	 * \param self this loader
	 * \param name the plugin name
	 * \param path the resolved path
	 * \return a new plugin
	 */
	struct irc_plugin * (*open)(struct irc_plugin_loader *self,
	                            const char *name,
	                            const char *path);

	/**
	 * (read-write, optional)
	 *
	 * Finalize the loader itself.
	 *
	 * The loader should not unload plugin by itself are this is
	 * automatically done by the bot itself.
	 *
	 * \param self this loader
	 */
	void (*finish)(struct irc_plugin_loader *self);

	/**
	 * \cond IRC_PRIVATE
	 */

	/**
	 * (private)
	 *
	 * Next plugin in the linked list.
	 */
	struct irc_plugin_loader *next;

	/**
	 * \endcond IRC_PRIVATE
	 */
};

/**
 * Initialize the plugin with predefined defaults.
 *
 * After this function is called, plugin metadata is set to the defaults from
 *
 * - ::IRC_PLUGIN_DEFAULT_LICENSE
 * - ::IRC_PLUGIN_DEFAULT_VERSION
 * - ::IRC_PLUGIN_DEFAULT_AUTHOR
 * - ::IRC_PLUGIN_DEFAULT_DESCRIPTION
 *
 * \pre plg != NULL
 * \pre name != NULL
 * \param plg the plugin to initialize
 * \param name the plugin name
 */
void
irc_plugin_init(struct irc_plugin *plg, const char *name);

/**
 * Set plugin metadata information.
 *
 * All strings are optional and are copied if not NULL
 *
 * \pre plg != NULL
 * \param license the plugin license
 * \param version the plugin version
 * \param author the plugin author
 * \param description the plugin description
 */
void
irc_plugin_set_info(struct irc_plugin *plg,
                    const char *license,
                    const char *version,
                    const char *author,
                    const char *description);

/**
 * \copydoc ::irc_plugin::set_template
 */
void
irc_plugin_set_template(struct irc_plugin *self, const char *key, const char *value);

/**
 * \copydoc ::irc_plugin::get_template
 */
const char *
irc_plugin_get_template(struct irc_plugin *self, const char *key);

/**
 * \copydoc ::irc_plugin::get_templates
 */
const char * const *
irc_plugin_get_templates(struct irc_plugin *self);

/**
 * \copydoc ::irc_plugin::set_path
 */
void
irc_plugin_set_path(struct irc_plugin *self, const char *key, const char *value);

/**
 * \copydoc ::irc_plugin::get_path
 */
const char *
irc_plugin_get_path(struct irc_plugin *self, const char *key);

/**
 * \copydoc ::irc_plugin::get_paths
 */
const char * const *
irc_plugin_get_paths(struct irc_plugin *self);

/**
 * \copydoc ::irc_plugin::set_option
 */
void
irc_plugin_set_option(struct irc_plugin *self, const char *key, const char *value);

/**
 * \copydoc ::irc_plugin::get_option
 */
const char *
irc_plugin_get_option(struct irc_plugin *self, const char *key);

/**
 * \copydoc ::irc_plugin::get_options
 */
const char * const *
irc_plugin_get_options(struct irc_plugin *self);

/**
 * \copydoc ::irc_plugin::load
 */
int
irc_plugin_load(struct irc_plugin *self);

/**
 * \copydoc ::irc_plugin::reload
 */
void
irc_plugin_reload(struct irc_plugin *self);

/**
 * \copydoc ::irc_plugin::unload
 */
void
irc_plugin_unload(struct irc_plugin *self);

/**
 * \copydoc ::irc_plugin::handle
 */
void
irc_plugin_handle(struct irc_plugin *self, const struct irc_event *ev);

/**
 * Finalize the plugin and invoke ::irc_plugin::finish function if not NULL.
 */
void
irc_plugin_finish(struct irc_plugin *self);

/**
 * Initialize the plugin loader.
 *
 * If not NULL, paths is a colon separated list of directories that will be used
 * when user wants to load a plugin without specifying a filesystem location.
 *
 * If not NULL, extensions is a colon separated list (without `.`) of extensions
 * that the plugin can have. This value is meaningless if paths is NULL.
 *
 * \pre ldr != NULL
 * \param paths optional paths to search
 * \param extensions optional extensions
 */
void
irc_plugin_loader_init(struct irc_plugin_loader *ldr,
                       const char *paths,
                       const char *extensions);

/**
 * \copydoc irc_plugin_loader::open
 */
struct irc_plugin *
irc_plugin_loader_open(struct irc_plugin_loader *self, const char *name, const char *path);

/**
 * Finalize the plugin and invoke ::irc_plugin_loader::finish function if not NULL.
 */
void
irc_plugin_loader_finish(struct irc_plugin_loader *self);

#if defined(__cplusplus)
}
#endif

#endif /* !IRCCD_PLUGIN_H */
