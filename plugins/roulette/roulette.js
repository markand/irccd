/*
 * roulette.js -- russian roulette game
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

var Logger = Irccd.Logger;
var Plugin = Irccd.Plugin;
var Server = Irccd.Server;
var Util = Irccd.Util;

/* Plugin information */
info = {
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "A russian roulette for IRC",
	version: "@IRCCD_VERSION@"
};

function Gun(server, channel)
{
	this.server = server;
	this.channel = channel;
	this.index = 0;
	this.bullet = Math.floor(Math.random() * 6);
}

/**
 * Map of games.
 */
Gun.map = {};

/**
 * Formats for writing.
 */
Gun.formats = {
	"lucky":	"#{nickname}, you're lucky this time",
	"shot":		"HEADSHOT"
};

/**
 * Search for an existing game.
 *
 * @param server the server object
 * @param channel the channel name
 * @return the hangman instance or undefined if no one exists
 */
Gun.find = function (server, channel)
{
	return Gun.map[server.toString() + '@' + channel];
}

/**
 * Create a new game, store it in the map and return it.
 *
 * @param server the server object
 * @param channel the channel name
 * @return the hangman object
 */
Gun.create = function (server, channel)
{
	return Gun.map[server.toString() + "@" + channel] = new Gun(server, channel);
}

/**
 * Remove the specified game from the map.
 *
 * @param game the game to remove
 */
Gun.remove = function (game)
{
	delete Gun.map[game.server + "@" + game.channel];
}

/**
 * Load all formats.
 */
Gun.loadFormats = function ()
{
	for (var key in Gun.formats) {
		var optname = "format-" + key;

		if (typeof (Plugin.config[optname]) !== "string")
			continue;

		if (Plugin.config[optname].length === 0)
			Logger.warning("skipping empty '" + optname + "' format");
		else
			Gun.formats[key] = Plugin.config[optname];
	}
}

Gun.prototype.shot = function ()
{
	return this.index++ === this.bullet;
}

function onLoad()
{
	Gun.loadFormats();
}

onReload = onLoad;

function onCommand(server, origin, channel)
{
	var kw = {
		channel: channel,
		command: server.info().commandChar + Plugin.info().name,
		nickname: Util.splituser(origin),
		origin: origin,
		server: server.toString(),
		plugin: Plugin.info().name
	};

	var game = Gun.find(server, channel);

	if (!game)
		game = Gun.create(server, channel);

	if (game.shot()) {
		server.kick(Util.splituser(origin), channel, Util.format(Gun.formats["shot"], kw));
		Gun.remove(game);
	} else {
		kw.count = (6 - game.index).toString();
		server.message(channel, Util.format(Gun.formats["lucky"], kw));
	}
}
