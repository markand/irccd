/*
 * history.js -- track nickname's history
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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
	summary: "track nickname's history",
	version: "@irccd_VERSION@"
};

// Modules.
var Directory = Irccd.Directory;
var File = Irccd.File;
var Logger = Irccd.Logger;
var Plugin = Irccd.Plugin;
var Server = Irccd.Server;
var Util = Irccd.Util;

Plugin.templates = {
	"error":        "#{nickname}, I'm sorry, something went wrong.",
	"seen":         "#{nickname}, I've seen #{target} for the last time the %d-%m-%Y %H:%M",
	"said":         "#{nickname}, #{target} said on %d-%m-%Y %H:%M: #{message}",
	"unknown":      "#{nickname}, I've never seen #{target}.",
	"usage":        "#{nickname}, usage: #{plugin} seen | said <target>."
};

function isSelf(server, origin)
{
	return server.info().nickname === Util.splituser(origin);
}

function command(server)
{
	return server.info().prefix + "history";
}

function path(server, channel)
{
	var p;

	if (Plugin.config["file"] !== undefined) {
		p = Util.format(Plugin.config["file"], {
			"server":       server.toString(),
			"channel":      channel
		});
	} else
		p = Plugin.paths.cache + "/db.json";

	return p;
}

function read(server, channel, nickname)
{
	var p = path(server, channel);
	var db = {};

	if (File.exists(p)) {
		var file = new File(path(server, channel), "r");
		var str = file.read();

		db = JSON.parse(str);
	}

	// Complete if needed.
	if (!db[server])
		db[server] = {};
	if (!db[server][channel])
		db[server][channel] = {};
	if (!db[server][channel][nickname])
		db[server][channel][nickname] = {};

	return db;
}

function write(server, channel, nickname, message)
{
	var db = read(server, channel, nickname);
	var entry = db[server][channel][nickname];
	var p = path(server, channel);

	if (!File.exists(File.dirname(p))) {
		Logger.debug("creating directory " + File.dirname(p));
		Directory.mkdir(File.dirname(p));
	}

	var file = new File(path(server, channel), "wt");

	entry.timestamp = Date.now();
	entry.message = (message) ? message : entry.message;

	file.write(JSON.stringify(db));
}

function find(server, channel, target)
{
	var db = read(server, channel, target);
	var it = db[server][channel][target];

	if (it.timestamp)
		return it;
}

function onCommand(server, origin, channel, message)
{
	channel = channel.toLowerCase();

	var args = message.trim().split(" ");
	var kw = {
		channel: channel,
		command: command(server),
		nickname: Util.splituser(origin),
		origin: origin,
		plugin: Plugin.info().name,
		server: server.toString()
	};

	if (args.length !== 2 || args[0].length === 0 || args[1].length === 0) {
		server.message(channel, Util.format(Plugin.templates.usage, kw));
		return;
	}

	if (args[0] !== "seen" && args[0] !== "said") {
		server.message(channel, Util.format(Plugin.templates.usage, kw));
		return;
	}

	args[1] = args[1].toLowerCase();

	if (isSelf(server, args[1]))
		return;

	try {
		var info = find(server, channel, args[1]);

		kw.target = args[1];

		if (!info) {
			server.message(channel, Util.format(Plugin.templates.unknown, kw));
			return;
		}

		kw.date = info.timestamp;
		kw.message = info.message ? info.message : "";

		server.message(channel, Util.format(Plugin.templates[args[0] == "seen" ? "seen" : "said"], kw));
	} catch (e) {
		server.message(channel, Util.format(Plugin.templates["error"], kw));
	}
}

function onJoin(server, origin, channel)
{
	origin = Util.splituser(origin).toLowerCase();
	channel = channel.toLowerCase();

	write(server, channel, origin);
}

function onMessage(server, origin, channel, message)
{
	origin = Util.splituser(origin).toLowerCase();
	channel = channel.toLowerCase();

	write(server, channel, origin, message);
}

onMe = onMessage;

function onTopic(server, origin, channel)
{
	origin = Util.splituser(origin).toLowerCase();
	channel = channel.toLowerCase();

	write(server, origin, channel)
}

function onLoad()
{
	/*
	 * If the plugin is loaded on-demand, we ask a name list for every
	 * server and every channel of them to update our database.
	 */
	var table = Server.list();

	for (var k in table) {
		var channels = table[k].info().channels;

		for (var i = 0; i < channels.length; ++i) {
			if (channels[i].joined)
				table[k].names(channels[i].name);
		}
	}
}

function onNames(server, channel, list)
{
	for (var i = 0; i < list.length; ++i)
		write(server, channel.toLowerCase(), list[i]);
}
