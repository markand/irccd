/*
 * logger.js -- plugin to log everything
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
    summary: "A plugin to log everything",
    version: "@IRCCD_VERSION@"
};

// Modules.
var Directory   = Irccd.Directory;
var File        = Irccd.File;
var Logger      = Irccd.Logger;
var Plugin      = Irccd.Plugin;
var Util        = Irccd.Util;

/**
 * All available formats.
 */
Plugin.format = {
    "cmode":    "%H:%M:%S :: #{nickname} changed the mode to: #{mode} #{arg}",
    "cnotice":  "%H:%M:%S :: [notice] (#{channel}) #{nickname}: #{message}",
    "join":     "%H:%M:%S >> #{nickname} joined #{channel}",
    "kick":     "%H:%M:%S :: #{target} has been kicked by #{nickname} [reason: #{reason}]",
    "me":       "%H:%M:%S * #{nickname} #{message}",
    "message":  "%H:%M:%S #{nickname}: #{message}",
    "mode":     "%H:%M:%S :: #{nickname} set mode #{mode} to #{arg}",
    "notice":   "%H:%M:%S [notice] (#{nickname}) #{message}",
    "part":     "%H:%M:%S << #{nickname} left #{channel} [#{reason}]",
    "query":    "%H:%M:%S #{nickname}: #{message}",
    "topic":    "%H:%M:%S :: #{nickname} changed the topic of #{channel} to: #{topic}"
};

/**
 * Load all formats.
 */
function loadFormats()
{
    // --- DEPRECATED -------------------------------------------
    //
    // This code will be removed.
    //
    // Since:    2.1.0
    // Until:    3.0.0
    // Reason:    new [format] section replaces it.
    //
    // ----------------------------------------------------------
    for (var key in Plugin.format) {
        var optname = "format-" + key;

        if (typeof (Plugin.config[optname]) !== "string")
            continue;

        if (Plugin.config[optname].length === 0)
            Logger.warning("skipping empty '" + optname + "' format");
        else
            Plugin.format[key] = Plugin.config[optname];
    }
}

function keywords(server, channel, origin, extra)
{
    var kw = {
        "server": server.toString(),
        "channel": channel,
        "origin": origin,
        "nickname": Util.splituser(origin)
    };

    for (var key in extra)
        kw[key] = extra[key];

    return kw;
}

function write(fmt, args)
{
    var path = Util.format(Plugin.config["path"], args);
    var directory = File.dirname(path);

    // Try to create the directory.
    if (!File.exists(directory)) {
        Logger.debug("creating directory: " + directory);
        Directory.mkdir(directory);
    }

    Logger.debug("opening: " + path);

    var str = Util.format(Plugin.format[fmt], args);
    var file = new File(path, "a");

    file.write(str + "\n");
}

function onLoad()
{
    if (Plugin.config["path"] === undefined)
        throw new Error("Missing 'path' option");

    loadFormats();
}

function onReload()
{
    loadFormats();
}

function onChannelMode(server, origin, channel, mode, arg)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("cmode", keywords(server, channel, origin, {
        "arg":      arg,
        "mode":     mode,
        "source":   channel
    }));
}

function onChannelNotice(server, origin, channel, notice)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("cnotice", keywords(server, channel, origin, {
        "message":  notice,
        "source":   channel
    }));
}

function onInvite(server, origin, channel)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("invite", keywords(server, channel, origin, {
        "source":   channel
    }));
}

function onJoin(server, origin, channel)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("join", keywords(server, channel, origin, {
        "source":   channel
    }));
}

function onKick(server, origin, channel, target, reason)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("kick", keywords(server, channel, origin, {
        "target":   target,
        "source":   channel,
        "reason":   reason
    }));
}

function onMe(server, origin, channel, message)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("me", keywords(server, channel, origin, {
        "message":  message,
        "source":   channel
    }));
}

function onMessage(server, origin, channel, message)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("message", keywords(server, channel, origin, {
        "message":  message,
        "source":   channel
    }));
}

function onMode(server, origin, mode)
{
    origin = origin.toLowerCase();

    write("mode", keywords(server, undefined, origin, {
        "mode":     mode,
        "source":   Util.splituser(origin)
    }));
}

function onNick(server, origin, nickname)
{
    // TODO: write for all servers/channels a log entry
}

function onNotice(server, origin, notice)
{
    origin = origin.toLowerCase();

    write("notice", keywords(server, undefined, origin, {
        "message":  notice,
        "source":   Util.splituser(origin)
    }));
}

function onPart(server, origin, channel, reason)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("part", keywords(server, channel, origin, {
        "reason":   reason,
        "source":   channel
    }));
}

function onQuery(server, origin, message)
{
    origin = origin.toLowerCase();

    write("query", keywords(server, undefined, origin, {
        "source":   Util.splituser(origin),
        "message":  message
    }));
}

function onTopic(server, origin, channel, topic)
{
    origin = origin.toLowerCase();
    channel = channel.toLowerCase();

    write("topic", keywords(server, channel, origin, {
        "source":   channel,
        "topic":    topic
    }));
}
