/*
 * hangman.js -- hangman game for IRC
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
    summary: "A hangman game for IRC",
    version: "@IRCCD_VERSION@"
};

// Modules.
var Logger = Irccd.Logger;
var File = Irccd.File;
var Plugin = Irccd.Plugin;
var Server = Irccd.Server;
var Unicode = Irccd.Unicode
var Util = Irccd.Util;

// Default options.
Plugin.config["collaborative"] = "true";

// Formats.
Plugin.format = {
    "asked":        "#{nickname}, '#{letter}' was already asked.",
    "dead":         "#{nickname}, fail the word was: #{word}.",
    "found":        "#{nickname}, nice! the word is now #{word}",
    "running":      "#{nickname}, the game is already running and the word is: #{word}",
    "start":        "#{nickname}, the game is started, the word to find is: #{word}",
    "win":          "#{nickname}, congratulations, the word is #{word}.",
    "wrong-word":   "#{nickname}, this is not the word.",
    "wrong-player": "#{nickname}, please wait until someone else proposes.",
    "wrong-letter": "#{nickname}, there is no '#{letter}'."
};

function Hangman(server, channel)
{
    this.server = server;
    this.channel = channel;
    this.tries = 10;
    this.select();
}

/**
 * Map of games.
 */
Hangman.map = {};

/**
 * List of words.
 */
Hangman.words = {
    all: [],        //!< All words,
    registry: {}    //!< Words list per server/channel.
};

/**
 * Search for an existing game.
 *
 * @param server the server object
 * @param channel the channel name
 * @return the hangman instance or undefined if no one exists
 */
Hangman.find = function (server, channel)
{
    return Hangman.map[server.toString() + '@' + channel];
}

/**
 * Create a new game, store it in the map and return it.
 *
 * @param server the server object
 * @param channel the channel name
 * @return the hangman object
 */
Hangman.create = function (server, channel)
{
    return Hangman.map[server.toString() + "@" + channel] = new Hangman(server, channel);
}

/**
 * Remove the specified game from the map.
 *
 * @param game the game to remove
 */
Hangman.remove = function (game)
{
    delete Hangman.map[game.server + "@" + game.channel];
}

/**
 * Check if the text is a valid word.
 *
 * @param word the word to check
 * @return true if a word
 */
Hangman.isWord = function (word)
{
    if (word.length === 0)
        return false;

    for (var i = 0; i < word.length; ++i)
        if (!Unicode.isLetter(word.charCodeAt(i)))
            return false;

    return true;
}

/**
 * Load all words.
 */
Hangman.loadWords = function ()
{
    var path;

    // User specified file?
    if (Plugin.config["file"])
        path = Plugin.config["file"];
    else
        path = Plugin.paths.config + "/words.conf";

    try {
        Logger.info("loading words...");

        var file = new File(path, "r");
        var line;

        while ((line = file.readline()) !== undefined)
            if (Hangman.isWord(line))
                Hangman.words.all.push(line);
    } catch (e) {
        throw new Error("could not open '" + path + "'");
    }

    if (Hangman.words.all.length === 0)
        throw new Error("empty word database");

    Logger.info("number of words in database: " + Hangman.words.all.length);
}

/**
 * Load all formats.
 */
Hangman.loadFormats = function ()
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

/**
 * Select the next word for the game.
 */
Hangman.prototype.select = function ()
{
    var id = this.server.toString() + "@" + this.channel;

    // Reload the words if empty.
    if (!Hangman.words.registry[id] || Hangman.words.registry[id].length === 0)
        Hangman.words.registry[id] = Hangman.words.all.slice(0);

    var i = Math.floor(Math.random() * Hangman.words.registry[id].length);

    this.word = Hangman.words.registry[id][i];

    // Erase words from the registry.
    Hangman.words.registry[id].splice(i, 1);

    // Fill table.
    this.table = {};

    for (var j = 0; j < this.word.length; ++j)
        this.table[this.word.charCodeAt(j)] = false;
}

/**
 * Format the word with underscore and letters.
 *
 * @return the secret
 */
Hangman.prototype.formatWord = function ()
{
    var str = "";

    for (var i = 0; i < this.word.length; ++i) {
        var ch = this.word.charCodeAt(i);

        if (!this.table[ch])
            str += "_";
        else
            str += String.fromCharCode(ch);

        if (i + 1 < this.word.length)
            str += " ";
    }

    return str;
}

/**
 * Propose a word or a letter.
 *
 * @param ch the code point or the unique word
 * @param nickname the user trying
 * @return the status of the game
 */
Hangman.prototype.propose = function (ch, nickname)
{
    var status = "found";

    // Check for collaborative mode.
    if (Plugin.config["collaborative"] === "true" && !this.query) {
        if (this.last !== undefined && this.last === nickname)
            return "wrong-player";

        this.last = nickname;
    }

    if (typeof(ch) == "number") {
        if (this.table[ch] === undefined) {
            this.tries -= 1;
            status = "wrong-letter";
        } else {
            if (this.table[ch]) {
                this.tries -= 1;
                status = "asked";
            } else
                this.table[ch] = true;
        }
    } else {
        if (this.word != ch) {
            this.tries -= 1;
            status = "wrong-word";
        } else
            status = "win";
    }

    // Check if dead.
    if (this.tries <= 0)
        status = "dead";

    // Check if win.
    var win = true;

    for (var i = 0; i < this.word.length; ++i) {
        if (!this.table[this.word.charCodeAt(i)]) {
            win = false;
            break;
        }
    }

    if (win)
        status = "win";

    return status;
}

function onLoad()
{
    Logger.warning("TAMERE");
    Hangman.loadFormats();
    Hangman.loadWords();
}

onReload = onLoad;

function propose(server, channel, origin, game, proposition)
{
    var kw = {
        channel: channel,
        command: server.info().commandChar + Plugin.info().name,
        nickname: Util.splituser(origin),
        origin: origin,
        plugin: Plugin.info().name,
        server: server.toString()
    };

    var st = game.propose(proposition, kw.nickname);

    switch (st) {
    case "found":
        kw.word = game.formatWord();
        server.message(channel, Util.format(Plugin.format["found"], kw));
        break;
    case "wrong-letter":
    case "wrong-player":
    case "wrong-word":
        kw.word = proposition;
    case "asked":
        kw.letter = String.fromCharCode(proposition);
        server.message(channel, Util.format(Plugin.format[st], kw));
        break;
    case "dead":
    case "win":
        kw.word = game.word;
        server.message(channel, Util.format(Plugin.format[st], kw));

        // Remove the game.
        Hangman.remove(game);
        break;
    default:
        break;
    }
}

function onCommand(server, origin, channel, message)
{
    channel = channel.toLowerCase();

    var game = Hangman.find(server, channel);
    var kw = {
        channel: channel,
        command: server.info().commandChar + Plugin.info().name,
        nickname: Util.splituser(origin),
        origin: origin,
        plugin: Plugin.info().name,
        server: server.toString()
    };

    if (game) {
        var list = message.split(" \t");

        if (list.length === 0 || String(list[0]).length === 0) {
            kw.word = game.formatWord();
            server.message(channel, Util.format(Plugin.format["running"], kw));
        } else {
            var word = String(list[0]);

            if (Hangman.isWord(word))
                propose(server, channel, origin, game, word);
        }
    } else {
        game = Hangman.create(server, channel);
        kw.word = game.formatWord();
        server.message(channel, Util.format(Plugin.format["start"], kw));
    }

    return game;
}

function onMessage(server, origin, channel, message)
{
    channel = channel.toLowerCase();

    var game = Hangman.find(server, channel);

    if (!game)
        return;

    if (message.length === 1 && Unicode.isLetter(message.charCodeAt(0)))
        propose(server, channel, origin, game, message.charCodeAt(0));
}

function onQueryCommand(server, origin, message)
{
    onCommand(server, origin, Util.splituser(origin), message).query = true;
}

function onQuery(server, origin, message)
{
    onMessage(server, origin, Util.splituser(origin), message);
}
