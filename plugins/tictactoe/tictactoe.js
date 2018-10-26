/*
 * tictactoe.js -- tictactoe game for IRC
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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
	name: "tictactoe",
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "A tictactoe game for IRC",
	version: "@IRCCD_VERSION@"
};

// Modules.
var Plugin = Irccd.Plugin;
var Util = Irccd.Util;

// Formats.
Plugin.format = {
	"draw":         "nobody won",
	"invalid":      "#{nickname}, please select a valid opponent",
	"running":      "#{nickname}, the game is already running",
	"turn":         "#{nickname}, it's your turn",
	"used":         "#{nickname}, this square is already used",
	"win":          "#{nickname}, congratulations, you won!"
};

/**
 * Create a game.
 *
 * This function creates a game without any checks.
 *
 * @param server the server object
 * @param channel the channel
 * @param origin the source origin
 * @param target the target nickname
 */
function Game(server, channel, origin, target)
{
	this.server = server;
	this.origin = origin;
	this.target = target;
	this.channel = channel;
	this.players = [ Util.splituser(origin), target ];
	this.player = Math.floor(Math.random() * 2);
	this.grid = [
		[ '.', '.', '.' ],
		[ '.', '.', '.' ],
		[ '.', '.', '.' ]
	];
}

// Pending games requests checking for names listing.
Game.requests = {};

// List of games running.
Game.map = {};

/**
 * Create a unique id.
 *
 * @param server the server object
 * @param channel the channel
 * @return the id
 */
Game.id = function (server, channel)
{
	return channel + "@" + server.toString();
}

/**
 * Get a running game or undefined.
 *
 * @param server the server object
 * @param channel the channel
 * @return the object or undefined if not running
 */
Game.find = function (server, channel)
{
	return Game.map[Game.id(server, channel)];
}

/**
 * Request a game after the name list gets received.
 *
 * @param server the server object
 * @param channel the channel
 * @param origin the originator
 * @return the object or undefined if not running
 */
Game.postpone = function (server, channel, origin, target)
{
	/*
	 * Get list of users on the channel to avoid playing against a non existing
	 * target.
	 */
	Game.requests[Game.id(server, channel)] = new Game(server, channel, origin, target);
	server.names(channel);
}

/**
 * Populate a set of keywords.
 *
 * @param server the server object
 * @param origin the originator
 * @param channel the channel
 * @return an object of predefined keywords
 */
Game.keywords = function (server, channel, origin)
{
	var kw = {
		channel: channel,
		command: server.info().commandChar + Plugin.info().name,
		plugin: Plugin.info().name,
		server: server.info().name
	};

	if (origin) {
		kw.origin = origin;
		kw.nickname = Util.splituser(origin);
	}

	return kw;
}

/**
 * Tells if a game is pending or running.
 *
 * @param server the server object
 * @param channel the channel
 * @return true if any
 */
Game.exists = function (server, channel)
{
	var id = Game.id(server, channel);

	return Game.requests[id] || Game.map[id];
}

/**
 * Delete a game from the registry.
 *
 * @param server the server object
 * @param channel the channel
 */
Game.remove = function (server, channel)
{
	delete Game.map[Game.id(server, channel)];
}

/**
 * Erase games when some players leave channels.
 *
 * @param server the server object
 * @param origin the originator
 * @param channel the channel
 */
Game.clear = function (server, user, channel)
{
	var nickname = Util.splituser(user);
	var game = Game.find(server, channel);

	if (game && (game.players[0] === nickname || game.players[1] === nickname))
		Game.remove(server, channel);
}

/**
 * Show the game grid and the next player line.
 */
Game.prototype.show = function ()
{
	var kw = Game.keywords(this.server, this.channel);

	// nickname is the current player.
	kw.nickname = this.players[this.player];

	this.server.message(this.channel, "  a b c");
	this.server.message(this.channel, "1 " + this.grid[0].join(" "));
	this.server.message(this.channel, "2 " + this.grid[1].join(" "));
	this.server.message(this.channel, "3 " + this.grid[2].join(" "));

	if (this.hasWinner())
		this.server.message(this.channel, Util.format(Plugin.format.win, kw));
	else if (this.hasDraw())
		this.server.message(this.channel, Util.format(Plugin.format.draw, kw));
	else
		this.server.message(this.channel, Util.format(Plugin.format.turn, kw));
}

/**
 * Tells if it's the nickname's turn.
 *
 * @param nickname the nickname to check
 * @return true if nickname is allowed to play
 */
Game.prototype.isTurn = function (nickname)
{
	return this.players[this.player] == nickname;
}

/**
 * Place the column and row as the current player.
 *
 * @param column the column (a, b or c)
 * @param row the row (1, 2 or 3)
 */
Game.prototype.place = function (column, row, origin)
{
	var columns = { a: 0, b: 1, c: 2 };
	var rows = { 1: 0, 2: 1, 3: 2 };

	column = columns[column];
	row = rows[row];

	var kw = Game.keywords(this.server, this.channel, origin);

	if (this.grid[row][column] !== '.') {
		this.server.message(this.channel, Util.format(Plugin.format.used, kw));
		return false;
	}

	this.grid[row][column] = this.player === 0 ? 'x' : 'o';

	// Do not change if game is finished.
	if (!this.hasWinner() && !this.hasDraw())
		this.player = this.player === 0 ? 1 : 0;

	return true;
}

/**
 * Check if there is a winner.
 *
 * @return true if there is a winner
 */
Game.prototype.hasWinner = function ()
{
	var lines = [
		[ [ 0, 0 ], [ 0, 1 ], [ 0, 2 ] ],
		[ [ 1, 0 ], [ 1, 1 ], [ 1, 2 ] ],
		[ [ 2, 0 ], [ 2, 1 ], [ 2, 2 ] ],
		[ [ 0, 0 ], [ 1, 0 ], [ 2, 0 ] ],
		[ [ 0, 1 ], [ 1, 1 ], [ 2, 1 ] ],
		[ [ 0, 2 ], [ 1, 2 ], [ 2, 2 ] ],
		[ [ 0, 0 ], [ 1, 1 ], [ 2, 2 ] ],
		[ [ 0, 2 ], [ 1, 1 ], [ 2, 0 ] ]
	];

	for (var i = 0; i < lines.length; ++i) {
		var p1 = lines[i][0];
		var p2 = lines[i][1];
		var p3 = lines[i][2];

		var result = this.grid[p1[0]][p1[1]] === this.grid[p2[0]][p2[1]] &&
		             this.grid[p2[0]][p2[1]] === this.grid[p3[0]][p3[1]] &&
		             this.grid[p3[0]][p3[1]] !== '.';

		if (result)
			return true;
	}
}

/**
 * Check if there is draw game.
 *
 * @return true if game is draw
 */
Game.prototype.hasDraw = function ()
{
	for (var r = 0; r < 3; ++r)
		for (var c = 0; c < 3; ++c)
			if (this.grid[r][c] === '.')
				return false;

	return true;
}

function onNames(server, channel, list)
{
	var id = Game.id(server, channel);
	var game = Game.requests[id];

	// Names can come from any other plugin/event.
	if (!game)
		return;

	// Not a valid target? destroy the game.
	if (list.indexOf(game.target) < 0)
		server.message(channel, Util.format(Plugin.format.invalid,
			Game.keywords(server, channel, game.origin)));
	else {
		Game.map[id] = game;
		game.show();
	}

	delete Game.requests[id];
}

function onCommand(server, origin, channel, message)
{
	var target = message.trim();
	var nickname = Util.splituser(origin);

	if (Game.exists(server, channel))
		server.message(channel, Util.format(Plugin.format.running, Game.keywords(server, channel, origin)));
	else if (target === "" || target === nickname || target === server.info().nickname)
		server.message(channel, Util.format(Plugin.format.invalid, Game.keywords(server, channel, origin)));
	else
		Game.postpone(server, channel, origin, message);
}

function onMessage(server, origin, channel, message)
{
	var nickname = Util.splituser(origin);
	var game = Game.find(server, channel);

	if (!game || !game.isTurn(nickname))
		return;

	var match = /^([abc]) ?([123])$/.exec(message.trim());

	if (!match)
		return;

	if (game.place(match[1], match[2], origin))
		game.show();
	if (game.hasWinner() || game.hasDraw())
		Game.remove(server, channel);
}

function onDisconnect(server)
{
	for (var key in Game.map)
		if (key.endsWith(server.toString()))
			delete Game.map[key];
}

function onKick(server, origin, channel, target)
{
	Game.clear(server, target, channel);
}

function onPart(server, origin, channel)
{
	Game.clear(server, origin, channel);
}
