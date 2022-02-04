/*
 * ask.js -- crazy module for asking a medium
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
	summary: "Crazy module for asking a medium",
	version: "@irccd_VERSION@"
};

// Modules.
var File = Irccd.File;
var Logger = Irccd.Logger;
var Plugin = Irccd.Plugin;
var Util = Irccd.Util;

/* List of answers */
var answers = [
	"Yes",
	"No"
];

function onLoad()
{
	try {
		// User specified file?
		if (Plugin.config["file"])
			path = Plugin.config["file"];
		else
			path = Plugin.paths.config + "/answers.conf";

		var file = new File(path, "r");
		var line;

		// Reset.
		answers = [];

		while ((line = file.readline()))
			// Skip empty lines.
			if (line.length > 0)
				answers.push(line);
	} catch (e) {
		Logger.warning(path + ": " + e.message);
		Logger.warning("using default answers");
	}
}

function onCommand(server, origin, channel)
{
	var target = Util.splituser(origin);
	var response = answers[Math.floor(Math.random() * answers.length)];

	server.message(channel, target + ", " + response);
}

onReload = onLoad;
