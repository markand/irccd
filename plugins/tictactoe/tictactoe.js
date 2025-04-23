/*
 * tictactoe.js -- tictactoe game for IRC
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

// Plugin information.
info = {
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "A tictactoe game for IRC",
	version: "@irccd_VERSION@"
};

// Modules.
var Plugin = Irccd.Plugin;
var Util = Irccd.Util;
var Timer = Irccd.Timer;

// Formats.
Plugin.templates = {
	"draw":         "Nobody won.",
	"invalid":      "#{nickname}, please select a valid opponent.",
	"running":      "#{nickname}, the game is already running.",
	"turn":         "#{nickname}, it's your turn.",
	"used":         "#{nickname}, this square is already used.",
	"win":          "#{nickname}, congratulations, you won!",
	"timeout":      "Aborted due to #{nickname} inactivity."
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
		command: server.info().prefix + Plugin.info().name,
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
 * Check if the target is valid.
 *
 * @param server the server object
 * @param channel the channel string
 * @param nickname the nickname who requested the game
 * @param target the opponent
 * @return true if target is valid
 */
Game.isValid = function (server, channel, nickname, target)
{
	if (target === "" || target === nickname || target === server.info().nickname)
		return false;

	var channels = server.info().channels;
	var ch;

	for (var i = 0; i < channels.length; ++i) {
		if (channels[i].name === channel) {
			ch = channels[i];
			break;
		}
	}

	for (var i = 0; i < ch.users.length; ++i)
		if (ch.users[i].nickname === target)
			return true;

	return false;
}

/**
 * Function called when a timeout occured.
 *
 * @param game the game to destroy
 */
Game.timeout = function (game)
{
	var kw = Game.keywords(game.server, game.channel);

	kw.nickname = game.players[game.player];
	game.server.message(game.channel, Util.format(Plugin.templates.timeout, kw));
	Game.remove(game.server, game.channel);
}

/**
 * Show the game grid and the next player line.
 */
Game.prototype.show = function ()
{
	var kw = Game.keywords(this.server, this.channel);
	var self = this;

	// nickname is the current player.
	kw.nickname = this.players[this.player];

	this.server.message(this.channel, "  a b c");
	this.server.message(this.channel, "1 " + this.grid[0].join(" "));
	this.server.message(this.channel, "2 " + this.grid[1].join(" "));
	this.server.message(this.channel, "3 " + this.grid[2].join(" "));

	if (this.hasWinner())
		this.server.message(this.channel, Util.format(Plugin.templates.win, kw));
	else if (this.hasDraw())
		this.server.message(this.channel, Util.format(Plugin.templates.draw, kw));
	else
		this.server.message(this.channel, Util.format(Plugin.templates.turn, kw));

	// Create a timer in case of inactivity (5 minutes).
	this.timer = new Irccd.Timer(Irccd.Timer.Single, 300000, function () {
		Game.timeout(self);
	});
	this.timer.start();
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
		this.server.message(this.channel, Util.format(Plugin.templates.used, kw));
		return false;
	}

	this.timer.stop();
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

function onCommand(server, origin, channel, message)
{
	channel = channel.toLowerCase();

	var target = message.trim();
	var nickname = Util.splituser(origin);

	if (Game.exists(server, channel))
		server.message(channel, Util.format(Plugin.templates.running, Game.keywords(server, channel, origin)));
	else if (!Game.isValid(server, channel, nickname, target))
		server.message(channel, Util.format(Plugin.templates.invalid, Game.keywords(server, channel, origin)));
	else {
		var game = new Game(server, channel, origin, target);

		Game.map[Game.id(server, channel)] = game;
		game.show();
	}
}

function onMessage(server, origin, channel, message)
{
	channel = channel.toLowerCase();

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
	Game.clear(server, target, channel.toLowerCase());
}

function onPart(server, origin, channel)
{
	Game.clear(server, origin, channel.toLowerCase());
}
