/*
 * test-jsapi-timer.c -- test Irccd.System API
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

/* TODO: We need proper bot function to dispatch */

#if 0

#define GREATEST_USE_ABBREVS 0
#include <greatest.h>

// TODO: irccd/
#include <config.h>

#include <irccd/js-plugin.h>
#include <irccd/plugin.h>

static struct irc_plugin *plugin;
static struct irc_js_plugin_data *data;

static void
setup(void *udata)
{
	(void)udata;

	plugin = irc_js_plugin_open(SOURCE "/data/timer.js");
	data = plugin->data;
}

static void
teardown(void *udata)
{
	(void)udata;

	irc_plugin_finish(plugin);

	plugin = NULL;
	data = NULL;
}

static void
set_type(const char *name)
{
	duk_get_global_string(data->ctx, "Irccd");
	duk_get_prop_string(data->ctx, -1, "Timer");
	duk_get_prop_string(data->ctx, -1, name.c_str());
	duk_put_global_string(data->ctx, "type");
	duk_pop_n(data->ctx, 2);

	plugin_->open();
	plugin_->handle_load(bot_);
}

BOOST_AUTO_TEST_CASE(single)
{
	boost::timer::cpu_timer timer;

	set_type("Single");

	while (timer.elapsed().wall / 1000000LL < 3000) {
		ctx_.reset();
		ctx_.poll();
	}

	BOOST_TEST(duk_get_global_string(data->ctx, "count"));
	BOOST_TEST(duk_get_int(data->ctx, -1) == 1);
}

BOOST_AUTO_TEST_CASE(repeat)
{
	boost::timer::cpu_timer timer;

	set_type("Repeat");

	while (timer.elapsed().wall / 1000000LL < 3000) {
		ctx_.reset();
		ctx_.poll();
	}

	BOOST_TEST(duk_get_global_string(data->ctx, "count"));
	BOOST_TEST(duk_get_int(data->ctx, -1) >= 5);
}

#endif

int
main(void)
{
	
}
