/*
 * plugin.js -- plugin inspector
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
	summary: "A plugin to inspect plugins",
	version: "@irccd_VERSION@"
};

// Modules.
var Util = Irccd.Util;
var Plugin = Irccd.Plugin;

Plugin.templates = {
	"usage":        "#{nickname}, usage: #{command} list | info plugin",
	"info":         "#{nickname}, #{name}: #{summary}, version #{version} by #{author} (#{license} license).",
	"not-found":    "#{nickname}, plugin #{name} does not exist.",
	"too-long":     "#{nickname}, plugin list too long, ask in query for more details."
}

Plugin.config = {
	"max-list-lines":   3,
	"max-list-columns": 80
}

var commands = {
	keywords: function (server, channel, origin)
	{
		return {
			channel: channel,
			command: server.info().prefix + Plugin.info().name,
			nickname: Util.splituser(origin),
			origin: origin,
			plugin: Plugin.info().name,
			server: server.toString()
		}
	},

	list: function (server, origin, target, query)
	{
		var kw = commands.keywords(server, target, origin);
		var plugins = Plugin.list();
		var lines = [ "" ];
		var maxl = parseInt(Plugin.config["max-list-lines"]);
		var maxc = parseInt(Plugin.config["max-list-columns"]);

		if (isNaN(maxc)) {
			maxc = 80;
		}
		if (isNaN(maxl)) {
			maxl = 3;
		}

		for (var p = 0; p < plugins.length; ++p) {
			var l = lines.length - 1;

			if (plugins[p].length + 1 + lines[l].length > maxc) {
				lines.push("");
				l++;
			}

			lines[l] += plugins[p] + " ";
		}

		if (!query && maxl > 0 && lines.length > maxl) {
			server.message(target, Util.format(Plugin.templates["too-long"], kw));
		} else {
			for (var i = 0; i < lines.length; ++i) {
				server.message(target, lines[i]);
			}
		}
	},

	info: function (server, origin, target, name)
	{
		var kw = commands.keywords(server, target, origin);
		var info = Plugin.info(name);
		var str;

		kw.name = name;

		if (info) {
			kw.author = info.author;
			kw.license = info.license;
			kw.summary = info.summary;
			kw.version = info.version;

			str = Util.format(Plugin.templates["info"], kw);
		} else
			str = Util.format(Plugin.templates["not-found"], kw);

		server.message(target, str);
	},

	usage: function (server, origin, target)
	{
		server.message(target, Util.format(Plugin.templates["usage"], commands.keywords(server, target, origin)));
	},

	execute: function (server, origin, target, message, query)
	{
		if (message.length === 0) {
			commands.usage(server, origin, target);
			return;
		}

		var list = message.split(" ");

		switch (list[0]) {
		case "info":
			if (list.length === 2)
				commands.info(server, origin, target, list[1]);
			else
				commands.usage(server, origin, target);

			break;
		case "list":
			commands.list(server, origin, target, query);
			break;
		default:
			commands.usage(server, origin, target);
			break;
		}
	}
};

function onCommand(server, origin, channel, message)
{
	commands.execute(server, origin, channel, message, false)
}

function onQueryCommand(server, origin, message)
{
	commands.execute(server, origin, origin, message, true)
}
