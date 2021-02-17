/*
 * This is a sample plugin in native C API.
 *
 * Note: we recommend that C plugins should be used as last resort because any
 * error will crash the whole daemon. It is also less convenient to share and
 * update.
 *
 * You can use CMake to build and install it to appropriate place using the
 * following code:
 *
 *   find_package(irccd REQUIRED)
 *   irccd_define_c_plugin(
 *     MAME myplugin
 *     SOURCES muplugin.c
 *   )
 *
 * You can also compile by hand using the pkg-config irccd file.
 *
 *   cc myplugin.c -o myplugin.so $(pkg-config --libs --cflags irccd)
 *
 * All symbols exported from the file must start with the plugin file basename
 * without its extension and with every non allowed character translated to
 * `_'. For example if the plugin is name `example-stuff' symbol must start
 * with `example_stuff_'. In this example we consider `example_`.
 */

/*
 * Include convention is using irccd/ prefix.
 *
 * The compat.h header contains additional BSD/POSIX extensions that may be
 * missing on your system. It is optional unless you explicitly use them.
 */

#include <string.h>

#include <irccd/compat.h>
#include <irccd/event.h>
#include <irccd/server.h>

/*
 * This is the plugin identifier, every variable are optional.
 */
const char *example_description = "Example of C plugin"
const char *example_version = "0.1.0";
const char *example_license = "ISC";
const char *example_author = "Name and optional email";

/*
 * get_options | get_templates | get_paths
 * ----------------------------------------------------------------------
 *
 * The following optional functions indicate to the daemon which keys are
 * supported as options, templates and paths respectively.
 *
 * Note: even if get_paths is not present or return NULL, irccd allows `cache',
 * `data' and `config' as standard keys.
 *
 * All three functions should return an array of strings which should be
 * terminated with a NULL value. They should not be dynamically allocated
 * because irccd does not assume they are.
 */

const char **
example_get_options(void)
{
	/* Indicate to irccd we support options `level' and `language' */
	static const char *keys[] = {
		"level",
		"language",
		NULL
	};

	return keys;
}

const char **
example_get_templates(void)
{
	/* Indicate to irccd we support templates `start' and `finish' */
	static const char *keys[] = {
		"start",
		"finish",
		NULL
	};

	return keys;
}

/*
 * get_option | get_template | get_path
 * ----------------------------------------------------------------------
 *
 * Those optional functions are analogous to their respective plural form
 * except they take a key as parameter.
 *
 * The plugin can receive an unknown key from the user, NULL can be returned if
 * they are not supported.
 *
 * The returned string isn't free'd by irccd so don't allocate any value
 * without storing it somewhere if really needed.
 */

const char *
example_get_option(const char *key)
{
	if (strcmp(key, "level") == 0)
		return "hard";
	else if (strcmp(key, "language") == 0)
		return "french";

	return NULL;
}

const char *
example_get_template(const char *key)
{
	if (strcmp(key, "start") == 0)
		return "#{nickname}, the game has started";
	else if (strcmp(key, "finish") == 0)
		return "#{nickname}, the game has finished";

	return NULL;
}

/*
 * set_option | set_template | set_path
 * ----------------------------------------------------------------------
 *
 * Finally the three functions are used to set a new value as options,
 * templates and paths respectively. Like their `get_*' counterpart, the plugin
 * may receive a unknown key from the user in that case it should be simply
 * ignored.
 *
 * Tip: the easiest to manage those is to use global fixed size strings.
 */

void
example_set_option(const char *key, const char *value)
{
	/* Assuming my_option_* variable exist. */
	if (strcmp(key, "level") == 0)
		strlcpy(my_option_level, value, sizeof (my_option_level));
	else if (strcmp(key, "language") == 0)
		strlcpy(my_option_language, value, sizeof (my_option_language));
}

void
example_set_template(const char *key)
{
	/* Assuming my_template_* variable exist. */
	if (strcmp(key, "level") == 0)
		strlcpy(my_template_level, value, sizeof (my_template_level));
	else if (strcmp(key, "language") == 0)
		strlcpy(my_template_language, value, sizeof (my_template_language));
}

/*
 * event
 * ----------------------------------------------------------------------
 *
 * This function is called when an event has been received. The parameter ev
 * contains a union with every possible supported event, the plugin must not
 * modify it.
 *
 * Use the ev->type enumeration to read the appropriate union member.
 */

void
example_event(const struct irc_event *ev)
{
	/* Simply echo back in case of message. */
	if (ev->type == IRC_EVENT_MESSAGE)
		irc_server_message(ev->server, ev->message.channel, ev->message.message);
}
