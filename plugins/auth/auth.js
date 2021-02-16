/*
 * auth.js -- generic plugin to authenticate to services
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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
	name: "auth",
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "Generic plugin to authenticate to services",
	version: "@IRCCD_VERSION@"
};

// Modules.
var Logger = Irccd.Logger;
var Plugin = Irccd.Plugin;
var Server = Irccd.Server;
var Util = Irccd.Util;

function authenticateNickserv(server, password)
{
	Logger.info("authenticating to NickServ on " + server.toString());

	var username = Plugin.config[server.toString() + ".username"];
	var str = Util.format("identify #{username}#{password}", {
		"username": username ? (username + " ") : "",
		"password": password
	});

	server.message("NickServ", str);
}

function authenticateQuakenet(server, password)
{
	var username = Plugin.config[server.toString() + ".username"];

	if (!username)
		Logger.warning("missing username for quakenet backend on " + server.toString());
	else {
		Logger.info("authenticating to Q on " + server.toString());
		server.message("Q@CServe.quakenet.org", Util.format("AUTH #{username} #{password}", {
			"username": username,
			"password": password
		}));
	}
}

function onConnect(server)
{
	var type = Plugin.config[server.toString() + ".type"];

	if (type) {
		// Password is mandatory.
		var password = Plugin.config[server.toString() + ".password"];

		if (!password)
			Logger.warning("missing password for " + server.toString());

		switch (type) {
		case "nickserv":
			authenticateNickserv(server, password);
			break;
		case "quakenet":
			authenticateQuakenet(server, password);
			break;
		default:
			Logger.warning("unknown '" + type + "' backend");
			break;
		}
	}
}
