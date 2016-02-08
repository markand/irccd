/*
 * history.js -- track nickname's history
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

/* Modules */
var Directory	= Irccd.Directory;
var File	= Irccd.File;
var Logger	= Irccd.Logger;
var Plugin	= Irccd.Plugin;
var Server	= Irccd.Server;
var Util	= Irccd.Util;

/* Plugin information */
info = {
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "track nickname's history",
	version: "@IRCCD_VERSION@"
};

var formats = {
	"error":	"#{nickname}, I'm sorry, something went wrong.",
	"seen":		"#{nickname}, I've seen #{target} for the last time the %d-%m-%Y %H:%M",
	"said":		"#{nickname}, #{target} said on %d-%m-%Y %H:%M: #{message}",
	"unknown":	"#{nickname}, I've never seen #{target}.",
	"usage":	"#{nickname}, usage: #{plugin} seen | said <target>."
};

function isSelf(server, origin)
{
	return server.info().nickname === Util.splituser(origin);
}

function command(server)
{
	return server.info().commandChar + "history";
}

function path(server, channel)
{
	var p;

	if (Plugin.config["path"] !== undefined) {
		p = Util.format(Plugin.config["path"], {
			"server":	server.toString(),
			"channel":	channel
		});
	} else {
		p = Plugin.cachePath + "db.json";
	}

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

	/* Complete if needed */
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

function loadFormats()
{
	for (var key in formats) {
		var optname = "format-" + key;

		if (typeof (Plugin.config[optname]) !== "string")
			continue;

		if (Plugin.config[optname].length === 0)
			Logger.warning("skipping empty '" + optname + "' format");
		else
			formats[key] = Plugin.config[optname];
	}
}

function onCommand(server, origin, channel, message)
{
	var args = message.trim().split(" ");
	var kw = {
		"server":	server.toString(),
		"channel":	channel,
		"origin":	origin,
		"nickname":	Util.splituser(origin),
		"plugin":	command(server),
	};

	if (args.length !== 2 || args[0].length === 0 || args[1].length === 0) {
		server.message(channel, Util.format(formats.usage, kw));
		return;
	}

	if (args[0] !== "seen" && args[0] !== "said") {
		server.message(channel, Util.format(formats.usage, kw));
		return;
	}

	if (isSelf(server, args[1]))
		return;

	try {
		var info = find(server, channel, args[1]);

		kw.target = args[1];

		if (!info) {
			server.message(channel, Util.format(formats.unknown, kw));
			return;
		}

		kw.date = info.timestamp;
		kw.message = info.message ? info.message : "";

		server.message(channel, Util.format(formats[args[0] == "seen" ? "seen" : "said"], kw));
	} catch (e) {
		server.message(channel, Util.format(kw));
	}
}

function onJoin(server, origin, channel)
{
	write(server, channel, Util.splituser(origin));
}

function onMessage(server, origin, channel, message)
{
	write(server, channel, Util.splituser(origin), message);
}

onMe = onMessage;

function onTopic(server, origin, channel)
{
	write(server, channel, Util.splituser(origin));
}

function onLoad()
{
	var table = Server.list();

	for (var k in table)
		for (var c in table[k].info().channels)
			table[k].names(c);

	loadFormats();
}

function onReload()
{
	loadFormats();
}

function onNames(server, channel, list)
{
	for (var i = 0; i < list.length; ++i)
		write(server, channel, list[i]);
}