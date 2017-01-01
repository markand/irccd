/*
 * plugin.js -- plugin inspector
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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
    version: "@IRCCD_VERSION@"
};

// Modules.
var Util = Irccd.Util;
var Plugin = Irccd.Plugin;

Plugin.format = {
    "usage":        "#{nickname}, usage: #{command} list | info plugin",
    "info":         "#{nickname}, #{name}: #{summary}, version #{version} by #{author} (#{license} license).",
    "not-found":    "#{nickname}, plugin #{name} does not exist.",
    "too-long":     "#{nickname}, plugin list too long, ask in query for more details."
}

var commands = {
    loadFormats: function ()
    {
        // --- DEPRECATED -----------------------------------
        //
        // This code will be removed.
        //
        // Since:    2.1.0
        // Until:    3.0.0
        // Reason:    new [format] section replaces it.
        //
        // --------------------------------------------------
        for (var key in Plugin.format) {
            var optname = "format-" + key;
    
            if (typeof (Plugin.config[optname]) !== "string")
                continue;
    
            if (Plugin.config[optname].length === 0)
                Logger.warning("skipping empty '" + optname + "' format");
            else
                Plugin.format[key] = Plugin.config[optname];
        }
    },
    
    keywords: function (server, channel, origin)
    {
        return {
            channel: channel,
            command: server.info().commandChar + Plugin.info().name,
            nickname: Util.splituser(origin),
            origin: origin,
            plugin: Plugin.info().name,
            server: server.toString()
        }
    },

    list: function (server, origin, target, query)
    {
        var kw = commands.keywords(server, target, origin);
        var list = Plugin.list();
        var str;

        if (!query && list.length >= 16)
            str = Util.format(Plugin.format["too-long"], kw);
        else
            str = list.join(" ");

        server.message(target, str);
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
    
            str = Util.format(Plugin.format["info"], kw);
        } else
            str = Util.format(Plugin.format["not-found"], kw);
    
        server.message(target, str);
    },

    usage: function (server, origin, target)
    {
        server.message(target, Util.format(Plugin.format["usage"], commands.keywords(server, target, origin)));
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

function onLoad()
{
    commands.loadFormats();
}

function onQueryCommand(server, origin, message)
{
    commands.execute(server, origin, origin, message, true)
}

function onReload()
{
    commands.loadFormats();
}
