/*
 * plugin.c -- abstract plugin
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

#include <assert.h>
#include <stdlib.h>

#include "plugin.h"

void
irc_plugin_set_template(struct irc_plugin *plg, const char *key, const char *value)
{
	assert(plg);
	assert(key);
	assert(value);

	if (plg->set_template)
		plg->set_template(plg, key, value);
}

const char *
irc_plugin_get_template(struct irc_plugin *plg, const char *key)
{
	assert(plg);
	assert(key);

	if (plg->get_template)
		return plg->get_template(plg, key);

	return NULL;
}

const char **
irc_plugin_get_templates(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->get_templates)
		return plg->get_templates(plg);

	return NULL;
}

void
irc_plugin_set_path(struct irc_plugin *plg, const char *key, const char *value)
{
	assert(plg);
	assert(key);
	assert(value);

	if (plg->set_path)
		plg->set_path(plg, key, value);
}

const char *
irc_plugin_get_path(struct irc_plugin *plg, const char *key)
{
	assert(plg);
	assert(key);

	if (plg->get_path)
		return plg->get_path(plg, key);

	return NULL;
}

const char **
irc_plugin_get_paths(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->get_paths)
		return plg->get_paths(plg);

	return NULL;
}

void
irc_plugin_set_option(struct irc_plugin *plg, const char *key, const char *value)
{
	assert(plg);
	assert(key);
	assert(value);

	if (plg->set_option)
		plg->set_option(plg, key, value);
}

const char *
irc_plugin_get_option(struct irc_plugin *plg, const char *key)
{
	assert(plg);
	assert(key);

	if (plg->get_option)
		return plg->get_option(plg, key);

	return NULL;
}

const char **
irc_plugin_get_options(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->get_options)
		return plg->get_options(plg);

	return NULL;
}

void
irc_plugin_load(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->load)
		plg->load(plg);
}

void
irc_plugin_reload(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->reload)
		plg->reload(plg);
}

void
irc_plugin_unload(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->unload)
		plg->unload(plg);
}

void
irc_plugin_handle(struct irc_plugin *plg, const struct irc_event *ev)
{
	assert(plg);
	assert(ev);

	if (plg->handle)
		plg->handle(plg, ev);
}

void
irc_plugin_finish(struct irc_plugin *plg)
{
	assert(plg);

	if (plg->finish)
		plg->finish(plg);

	free(plg);
}
