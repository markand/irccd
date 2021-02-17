/*
 * This is a sample plugin in Javascript API.
 */

/*
 * This is the plugin identifier, every variable are optional.
 */
info = {
	author: "David Demelier <markand@malikania.fr>",
	license: "ISC",
	summary: "Crazy module for asking a medium",
	version: "@IRCCD_VERSION@"
};

/*
 * Called when the user invoke the plugin using its identifier and the server
 * prefix.
 *
 * Example: !example foo bar baz
 */
function onCommand(server, origin, channel, message)
{
}

/*
 * Called when a server successfully connect and identifies to a IRC server.
 */
function onConnect(server)
{
}

/*
 * Called when a server disconnection is detected.
 */
function onDisconnect(server)
{
}

/*
 * Called when someone invites the bot on a channel.
 */
function onInvite(server, origin, channel)
{
}

/*
 * Called when someones join a channel (the bot included).
 */
function onJoin(server, origin, channel)
{
}

/*
 * Called when a someone was kicked from a channel.
 */
function onKick(server, origin, channel, reason)
{
}

/*
 * Called when a plugin is being loaded. Never happens from IRC.
 */
function onLoad()
{
}

/*
 * Called when a special CTCP ACTION (/me) is received.
 */
function onMe(server, origin, channel, message)
{
}

/*
 * Called when a message has been received.
 */
function onMessage(server, origin, channel, message)
{
}

/*
 * Called when a user/channel mode change. The channel can be the bot nickname.
 * The args is a list of string containing mode arguments.
 */
function onMode(server, origin, channel, args)
{
}

/*
 * Called when a list of names have been received.
 *
 * Note: in contrast to the IRC names listing, the names are not prefixed with
 * their optional channel mode (e.g. @+ etc).
 *
 * Tip: using this event is no longer necessary starting from irccd 4, the bot
 * keeps track of users of every channel it is present and can be accessed
 * through the method Irccd.Server.prototype.info.
 */
function onNames(server, channel, names)
{
}

/*
 * Called when a nickname change.
 */
function onNick(server, origin, nickname)
{
}

/*
 * Called when a notice is received. The channel can be the bot nickname as
 * well.
 */
function onNotice(server, origin, channel, notice)
{
}

/*
 * Called when someone leaves a channel.
 */
function onPart(server, origin, channel, reason)
{
}

/*
 * Called when the user request a plugin reload. Never happens from IRC.
 */
function onReload()
{
}

/*
 * Called when a topic change.
 */
function onTopic(server, origin, channel, topic)
{
}

/*
 * Called when the plugin is about to be removed. Never happens from IRC.
 */
function onUnload()
{
}

/*
 * Called when a whois information has been received. This function is usually
 * never called unless the plugin explicitly calls Irccd.Server.prototype.whois
 * beforehand.
 */
function onWhois(server, info)
{
}
