/*
 * joke.js -- display some jokes
 *
 * Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

// Plugin information.
info = {
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "display some jokes",
	version: "@irccd_VERSION@"
};

// Modules.
var File = Irccd.File;
var Logger = Irccd.Logger;
var Plugin = Irccd.Plugin;
var Util = Irccd.Util;

Plugin.config["max-list-lines"] = 5;
Plugin.templates["error"] = "#{nickname}: no jokes available";

/**
 * Map of server/channel to list of jokes.
 *
 * Jokes consists of array of array of strings, they are printed line-per-line
 * on the desired server/channel.
 *
 * This registry keeps track of jokes that were already printed to avoid sending
 * the same joke over and over.
 *
 * It is implemented like this:
 *
 * ```json
 * {
 *  "channel1@server1": [
 *      [
 *          "line 1 of joke 1"
 *          "line 2 of joke 1"
 *      ],
 *      [
 *          "line 1 of joke 2"
 *          "line 2 of joke 2"
 *      ]
 *  },
 *  "channel2@server2": [
 *      [
 *          "line 1 of joke 1"
 *          "line 2 of joke 1"
 *      ],
 *      [
 *          "line 1 of joke 2"
 *          "line 2 of joke 2"
 *      ]
 *  },
 * };
 * ```
 */
var table = {};

/**
 * Load the jokes for this server and channel.
 *
 * Jokes that are to big to fit into the max-list-lines parameter are discarded.
 *
 * \param server the server object
 * \param channel the channel string
 * \return a ready to use array of jokes
 * \throw Error if
 */
function load(server, channel)
{
	var path = Plugin.config.file
		? Plugin.config.file
		: Plugin.paths.data + "/jokes.json";

	// Allow formatting to select different jokes per server/channel.
	path = Util.format(path, {
		server: server.toString(),
		channel: channel
	});

	try {
		var file = new File(path, "r");
		var data = JSON.parse(file.read());
	} catch (e) {
		throw Error(path + ": " + e.message);
	}

	if (!data || !data.length)
		throw Error(path + ": no jokes found");

	// Ensure that jokes only contain strings.
	var jokes = data.filter(function (joke) {
		if (!joke || joke.length == 0 || joke.length > parseInt(Plugin.config["max-list-lines"]))
			return false;

		for (var i = 0; i < joke.length; ++i)
			if (typeof (joke[i]) !== "string")
				return false;

		return true;
	});

	if (!jokes || jokes.length === 0)
		throw Error(path + ": empty jokes");

	return jokes;
}

/**
 * Convert a pair server/channel into a unique identifier.
 *
 * \return channel@server
 */
function id(server, channel)
{
	return channel + "@" + server.toString();
}

/**
 * Show the joke in the specified channel.
 *
 * \warning this function does not check for max-list-lines parameter
 * \param server the server object
 * \param channel the channel string
 * \param joke the joke array (array of strings)
 */
function show(server, channel, joke)
{
	for (var l = 0; l < joke.length; ++l)
		server.message(channel, joke[l]);
}

/**
 * Remove the joke from the table.
 *
 * \param i the server/channel identifier
 * \param index the joke index
 */
function remove(i, index)
{
	table[i].splice(index, 1);

	if (table[i].length == 0)
		delete table[i];
}

function onCommand(server, origin, channel, message)
{
	var i = id(server, channel);

	if (!table[i]) {
		Logger.debug("reloading for " + i);

		try {
			table[i] = load(server, channel);
		} catch (e) {
			Logger.warning(e.message);
			server.message(channel, Util.format(Plugin.templates.error, {
				plugin: Plugin.info().name,
				command: server.info().prefix + Plugin.info().name,
				server: server.toString(),
				channel: channel,
				origin: origin,
				nickname: Util.splituser(origin)
			}));

			return;
		}
	}

	var index = Math.floor(Math.random() * table[i].length);

	show(server, channel, table[i][index]);
	remove(i, index);
}

function onReload()
{
	// This will force reload of jokes on next onCommand.
	table = {};
}
