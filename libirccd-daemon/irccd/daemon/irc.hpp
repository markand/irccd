/*
 * irc.hpp -- low level IRC functions
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_IRC_HPP
#define IRCCD_IRC_HPP

/**
 * \file irc.hpp
 * \brief Low level IRC functions.
 */

#include <irccd/sysconfig.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/asio.hpp>

#if defined(IRCCD_HAVE_SSL)
#	include <boost/asio/ssl.hpp>
#endif

namespace irccd::daemon::irc {

/**
 * \brief Describe errors.
 *
 * See [RFC1459 (6.1)](https://tools.ietf.org/html/rfc1459#section-6.1).
 */
enum class err {
	/**
	 * ERR_NOSUCHNICK
	 *
	 * "<nickname> :No such nick/channel"
	 *
	 * Used to indicate the nickname parameter supplied to a
	 * command is currently unused.
	 */
	nosuchnick = 401,

	/**
	 * ERR_NOSUCHSERVER
	 *
	 * "<server name> :No such server"
	 *
	 * Used to indicate the server name given currently
	 * doesn't exist.
	 */
	nosuchserver = 402,

	/**
	 * ERR_NOSUCHCHANNEL
	 *
	 * "<channel name> :No such channel"
	 *
	 * Used to indicate the given channel name is invalid.
	 */
	nosuchchannel = 403,

	/**
	 * ERR_CANNOTSENDTOCHAN
	 *
	 * "<channel name> :Cannot send to channel"
	 *
	 * Sent to a user who is either (a) not on a channel
	 * which is mode +n or (b) not a chanop (or mode +v) on
	 * a channel which has mode +m set and is trying to send
	 * a PRIVMSG message to that channel.
	 */
	cannotsendtochan = 404,

	/**
	 * ERR_TOOMANYCHANNELS
	 *
	 * "<channel name> :You have joined too many channels"
	 *
	 * Sent to a user when they have joined the maximum
	 * number of allowed channels and they try to join
	 * another channel.
	 */
	toomanychannels = 405,

	/**
	 * ERR_WASNOSUCHNICK
	 *
	 * "<nickname> :There was no such nickname"
	 *
	 * Returned by WHOWAS to indicate there is no history
	 * information for that nickname.
	 */
	wasnosuchnick = 406,

	/**
	 * ERR_TOOMANYTARGETS
	 *
	 * "<target> :Duplicate recipients. No message delivered"
	 *
	 * Returned to a client which is attempting to send a
	 * PRIVMSG/NOTICE using the user@host destination format
	 * and for a user@host which has several occurrences.
	 */
	toomanytargets = 407,

	/**
	 * ERR_NOORIGIN
	 *
	 * ":No origin specified"
	 *
	 * PING or PONG message missing the originator parameter
	 * which is required since these commands must work
	 * without valid prefixes.
	 */
	noorigin = 409,

	/**
	 * ERR_NORECIPIENT
	 *
	 * ":No recipient given (<command>)"
	 */
	norecipient = 411,

	/**
	 * ERR_NOTEXTTOSEND
	 *
	 * ":No text to send"
	 */
	notexttosend = 412,

	/**
	 * ERR_NOTOPLEVEL
	 *
	 * "<mask> :No toplevel domain specified"
	 */
	notoplevel = 413,

	/**
	 * ERR_WILDTOPLEVEL
	 *
	 * "<mask> :Wildcard in toplevel domain"
	 *
	 * are returned by PRIVMSG to indicate that
	 * the message wasn't delivered for some reason.
	 * ERR_NOTOPLEVEL and ERR_WILDTOPLEVEL are errors that
	 * are returned when an invalid use of
	 * "PRIVMSG $<server>" or "PRIVMSG #<host>" is attempted.
	 */
	wildtoplevel = 414,

	/**
	 * ERR_UNKNOWNCOMMAND
	 *
	 * "<command> :Unknown command"
	 *
	 * Returned to a registered client to indicate that the
	 * command sent is unknown by the server.
	 */
	unknowncommand = 421,

	/**
	 * ERR_NOMOTD
	 *
	 * ":MOTD File is missing"
	 *
	 * Server's MOTD file could not be opened by the server.
	 */
	nomotd = 422,

	/**
	 * ERR_NOADMININFO
	 *
	 * "<server> :No administrative info available"
	 *
	 * Returned by a server in response to an ADMIN message
	 * when there is an error in finding the appropriate
	 * information.
	 */
	noadmininfo = 423,

	/**
	 * ERR_FILEERROR
	 *
	 * ":File error doing <file op> on <file>"
	 *
	 * Generic error message used to report a failed file
	 * operation during the processing of a message.
	 */
	fileerror = 424,

	/**
	 * ERR_NONICKNAMEGIVEN
	 *
	 * ":No nickname given"
	 *
	 * Returned when a nickname parameter expected for a
	 * command and isn't found.
	 */
	nonicknamegiven = 431,

	/**
	 * ERR_ERRONEUSNICKNAME
	 *
	 * "<nick> :Erroneus nickname"
	 *
	 * Returned after receiving a NICK message which contains
	 * characters which do not fall in the defined set.  See
	 * section x.x.x for details on valid nicknames.
	 */
	erroneusnickname = 432,

	/**
	 * ERR_NICKNAMEINUSE
	 *
	 * "<nick> :Nickname is already in use"
	 *
	 * Returned when a NICK message is processed that results
	 * in an attempt to change to a currently existing
	 * nickname.
	 */
	nicknameinuse = 433,

	/**
	 * ERR_NICKCOLLISION
	 *
	 * "<nick> :Nickname collision KILL"
	 *
	 * Returned by a server to a client when it detects a
	 * nickname collision (registered of a NICK that
	 * already exists by another server).
	 */
	nickcollision = 436,

	/**
	 * ERR_USERNOTINCHANNEL
	 *
	 * "<nick> <channel> :They aren't on that channel"
	 *
	 * Returned by the server to indicate that the target
	 * user of the command is not on the given channel.
	 */
	usernotinchannel = 441,

	/**
	 * ERR_NOTONCHANNEL
	 *
	 * "<channel> :You're not on that channel"
	 *
	 * Returned by the server whenever a client tries to
	 * perform a channel effecting command for which the
	 * client isn't a member.
	 */
	notonchannel = 442,

	/**
	 * ERR_USERONCHANNEL
	 *
	 * "<user> <channel> :is already on channel"
	 *
	 * Returned when a client tries to invite a user to a
	 * channel they are already on.
	 */
	useronchannel = 443,

	/**
	 * ERR_NOLOGIN
	 *
	 * "<user> :User not logged in"
	 *
	 * Returned by the summon after a SUMMON command for a
	 * user was unable to be performed since they were not
	 * logged in.
	 */
	nologin = 444,

	/**
	 * ERR_SUMMONDISABLED
	 *
	 * ":SUMMON has been disabled"
	 *
	 * Returned as a response to the SUMMON command.  Must be
	 * returned by any server which does not implement it.
	 */
	summondisabled = 445,

	/**
	 * ERR_USERSDISABLED
	 *
	 * ":USERS has been disabled"
	 *
	 * Returned as a response to the USERS command.  Must be
	 * returned by any server which does not implement it.
	 */
	usersdisabled = 446,

	/**
	 * ERR_NOTREGISTERED
	 *
	 * ":You have not registered"
	 *
	 * Returned by the server to indicate that the client
	 * must be registered before the server will allow it
	 * to be parsed in detail.
	 */
	notregistered = 451,

	/**
	 * ERR_NEEDMOREPARAMS
	 *
	 * "<command> :Not enough parameters"
	 *
	 * Returned by the server by numerous commands to
	 * indicate to the client that it didn't supply enough
	 * parameters.
	 */
	needmoreparams = 461,

	/**
	 * ERR_ALREADYREGISTRED
	 *
	 * ":You may not reregister"
	 *
	 * Returned by the server to any link which tries to
	 * change part of the registered details (such as
	 * password or user details from second USER message).
	 */
	alreadyregistred = 462,

	/**
	 * ERR_NOPERMFORHOST
	 *
	 * ":Your host isn't among the privileged"
	 *
	 * Returned to a client which attempts to register with
	 * a server which does not been setup to allow
	 * connections from the host the attempted connection
	 * is tried.
	 */
	nopermforhost = 463,

	/**
	 * ERR_PASSWDMISMATCH
	 *
	 * ":Password incorrect"
	 *
	 * Returned to indicate a failed attempt at registering
	 * a connection for which a password was required and
	 * was either not given or incorrect.
	 */
	passwdmismatch = 464,

	/**
	 * ERR_YOUREBANNEDCREEP
	 *
	 * ":You are banned from this server"
	 *
	 * Returned after an attempt to connect and register
	 * yourself with a server which has been setup to
	 * explicitly deny connections to you.
	 */
	yourebannedcreep = 465,

	/**
	 * ERR_KEYSET
	 *
	 * "<channel> :Channel key already set"
	 */
	keyset = 467,

	/**
	 * ERR_CHANNELISFULL
	 *
	 * "<channel> :Cannot join channel (+l)"
	 */
	channelisfull = 471,

	/**
	 * ERR_UNKNOWNMODE
	 *
	 * "<char> :is unknown mode char to me"
	 */
	unknownmode = 472,

	/**
	 * ERR_INVITEONLYCHAN
	 *
	 * "<channel> :Cannot join channel (+i)"
	 */
	inviteonlychan = 473,

	/**
	 * ERR_BANNEDFROMCHAN
	 *
	 * "<channel> :Cannot join channel (+b)"
	 */
	bannedfromchan = 474,

	/**
	 * ERR_BADCHANNELKEY
	 *
	 * "<channel> :Cannot join channel (+k)"
	 */
	badchannelkey = 475,

	/**
	 * ERR_NOPRIVILEGES
	 *
	 * ":Permission Denied- You're not an IRC operator"
	 *
	 * Any command requiring operator privileges to operate
	 * must return this error to indicate the attempt was
	 * unsuccessful.
	 */
	noprivileges = 481,

	/**
	 * ERR_CHANOPRIVSNEEDED
	 *
	 * "<channel> :You're not channel operator"
	 *
	 * Any command requiring 'chanop' privileges (such as
	 * MODE messages) must return this error if the client
	 * making the attempt is not a chanop on the specified
	 * channel.
	 */
	chanoprivsneeded = 482,

	/**
	 * ERR_CANTKILLSERVER
	 *
	 * ":You cant kill a server!"
	 *
	 * Any attempts to use the KILL command on a server
	 * are to be refused and this error returned directly
	 * to the client.
	 */
	cantkillserver = 483,

	/**
	 * ERR_NOOPERHOST
	 *
	 * ":No O-lines for your host"
	 *
	 * If a client sends an OPER message and the server has
	 * not been configured to allow connections from the
	 * client's host as an operator, this error must be
	 * returned.
	 */
	nooperhost = 491,

	/**
	 * ERR_UMODEUNKNOWNFLAG
	 *
	 * ":Unknown MODE flag"
	 *
	 * Returned by the server to indicate that a MODE
	 * message was sent with a nickname parameter and that
	 * the a mode flag sent was not recognized.
	 */
	umodeunknownflag = 501,

	/**
	 * ERR_USERSDONTMATCH
	 *
	 * ":Cant change mode for other users"
	 *
	 * Error sent to any user trying to view or change the
	 * user mode for a user other than themselves.
	 */
	usersdontmatch = 502
};

/**
 * \brief Describe numeric replies.
 *
 * See [RFC1459 (6.2)](https://tools.ietf.org/html/rfc1459#section-6.2).
 */
enum class rpl {
	/**
	 * RPL_NONE
	 *
	 * Dummy reply number. Not used.
	 */
	none = 300,

	/**
	 * RPL_USERHOST
	 *
	 * ":[<reply>{<space><reply>}]"
	 *
	 * Reply format used by USERHOST to list replies to
	 * the query list.  The reply string is composed as
	 * follows:
	 *
	 * <reply> ::= <nick>['*'] '=' <'+'|'-'><hostname>
	 *
	 * The '*' indicates whether the client has registered
	 * as an Operator.  The '-' or '+' characters represent
	 * whether the client has set an AWAY message or not
	 * respectively.
	 */
	userhost = 302,

	/**
	 * RPL_ISON
	 *
	 * ":[<nick> {<space><nick>}]"
	 *
	 * Reply format used by ISON to list replies to the
	 * query list.
	 */
	ison = 303,

	/**
	 * RPL_AWAY
	 *
	 * "<nick> :<away message>"
	 */
	away = 301,

	/**
	 * RPL_UNAWAY
	 *
	 * ":You are no longer marked as being away"
	 */
	unaway = 305,

	/**
	 * RPL_NOWAWAY
	 *
	 * ":You have been marked as being away"
	 *
	 * These replies are used with the AWAY command (if
	 * allowed).  RPL_AWAY is sent to any client sending a
	 * PRIVMSG to a client which is away.  RPL_AWAY is only
	 * sent by the server to which the client is connected.
	 * Replies RPL_UNAWAY and RPL_NOWAWAY are sent when the
	 * client removes and sets an AWAY message.
	 */
	nowaway = 306,

	/**
	 * RPL_WHOISUSER
	 *
	 * "<nick> <user> <host> * :<real name>"
	 */
	whoisuser = 311,

	/**
	 * RPL_WHOISSERVER
	 *
	 * "<nick> <server> :<server info>"
	 */
	whoisserver = 312,

	/**
	 * RPL_WHOISOPERATOR
	 *
	 * "<nick> :is an IRC operator"
	 */
	whoisoperator = 313,

	/**
	 * RPL_WHOISIDLE
	 *
	 * "<nick> <integer> :seconds idle"
	 */
	whoisidle = 317,

	/**
	 * RPL_ENDOFWHOIS
	 *
	 * "<nick> :End of /WHOIS list"
	 */
	endofwhois = 318,

	/**
	 * RPL_WHOISCHANNELS
	 *
	 * "<nick> :{[@|+]<channel><space>}"
	 *
	 * Replies 311 - 313, 317 - 319 are all replies
	 * generated in response to a WHOIS message.  Given that
	 * there are enough parameters present, the answering
	 * server must either formulate a reply out of the above
	 * numerics (if the query nick is found) or return an
	 * error reply.  The '*' in RPL_WHOISUSER is there as
	 * the literal character and not as a wild card.  For
	 * each reply set, only RPL_WHOISCHANNELS may appear
	 * more than once (for long lists of channel names).
	 * The '@' and '+' characters next to the channel name
	 * indicate whether a client is a channel operator or
	 * has been granted permission to speak on a moderated
	 * channel.  The RPL_ENDOFWHOIS reply is used to mark
	 * the end of processing a WHOIS message.
	 */
	whoischannels = 319,

	/**
	 * RPL_WHOWASUSER
	 *
	 * "<nick> <user> <host> * :<real name>"
	 */
	whowasuser = 314,

	/**
	 * RPL_ENDOFWHOWAS
	 *
	 * "<nick> :End of WHOWAS"
	 *
	 * When replying to a WHOWAS message, a server must use
	 * the replies RPL_WHOWASUSER, RPL_WHOISSERVER or
	 * ERR_WASNOSUCHNICK for each nickname in the presented
	 * list.  At the end of all reply batches, there must
	 * be RPL_ENDOFWHOWAS (even if there was only one reply
	 * and it was an error).
	 */
	endofwhowas = 369,

	/**
	 * RPL_LISTSTART
	 *
	 * "Channel :Users  Name"
	 */
	liststart = 321,

	/**
	 * RPL_LIST
	 *
	 * "<channel> <# visible> :<topic>"
	 */
	list = 322,

	/**
	 * RPL_LISTEND
	 *
	 * ":End of /LIST"
	 *
	 * Replies RPL_LISTSTART, RPL_LIST, RPL_LISTEND mark
	 * the start, actual replies with data and end of the
	 * server's response to a LIST command.  If there are
	 * no channels available to return, only the start
	 * and end reply must be sent.
	 */
	listend = 323,

	/**
	 * RPL_CHANNELMODEIS
	 *
	 * "<channel> <mode> <mode params>"
	 */
	channelmodeis = 324,

	/**
	 * RPL_NOTOPIC
	 *
	 * "<channel> :No topic is set"
	 */
	notopic = 331,

	/**
	 * RPL_TOPIC
	 *
	 * "<channel> :<topic>"
	 *
	 * When sending a TOPIC message to determine the
	 * channel topic, one of two replies is sent.  If
	 * the topic is set, RPL_TOPIC is sent back else
	 * RPL_NOTOPIC.
	 */
	topic = 332,

	/**
	 * RPL_INVITING
	 *
	 * "<channel> <nick>"
	 *
	 * Returned by the server to indicate that the
	 * attempted INVITE message was successful and is
	 * being passed onto the end client.
	 */
	inviting = 341,

	/**
	 * RPL_SUMMONING
	 *
	 * "<user> :Summoning user to IRC"
	 *
	 * Returned by a server answering a SUMMON message to
	 * indicate that it is summoning that user.
	 */
	summoning = 342,

	/**
	 * RPL_VERSION
	 *
	 * "<version>.<debuglevel> <server> :<comments>"
	 *
	 * Reply by the server showing its version details.
	 * The <version> is the version of the software being
	 * used (including any patchlevel revisions) and the
	 * <debuglevel> is used to indicate if the server is
	 * running in "debug mode".
	 *
	 * The "comments" field may contain any comments about
	 * the version or further version details.
	 */
	version = 351,

	/**
	 * RPL_WHOREPLY
	 *
	 * "<channel> <user> <host> <server> <nick> \
	 *  <H|G>[*][@|+] :<hopcount> <real name>"
	 */
	whoreply = 352,

	/**
	 * RPL_ENDOFWHO
	 *
	 * "<name> :End of /WHO list"
	 *
	 * The RPL_WHOREPLY and RPL_ENDOFWHO pair are used
	 * to answer a WHO message.  The RPL_WHOREPLY is only
	 * sent if there is an appropriate match to the WHO
	 * query.  If there is a list of parameters supplied
	 * with a WHO message, a RPL_ENDOFWHO must be sent
	 * after processing each list item with <name> being
	 * the item.
	 */
	endofwho = 315,

	/**
	 * RPL_NAMREPLY
	 *
	 * "<channel> :[[@|+]<nick> [[@|+]<nick> [...]]]"
	 */
	namreply = 353,

	/**
	 * RPL_ENDOFNAMES
	 *
	 * "<channel> :End of /NAMES list"
	 *
	 * To reply to a NAMES message, a reply pair consisting
	 * of RPL_NAMREPLY and RPL_ENDOFNAMES is sent by the
	 * server back to the client.  If there is no channel
	 * found as in the query, then only RPL_ENDOFNAMES is
	 * returned.  The exception to this is when a NAMES
	 * message is sent with no parameters and all visible
	 * channels and contents are sent back in a series of
	 * RPL_NAMEREPLY messages with a RPL_ENDOFNAMES to mark
	 * the end.
	 */
	endofnames = 366,

	/**
	 * RPL_LINKS
	 *
	 * "<mask> <server> :<hopcount> <server info>"
	 */
	links = 364,

	/**
	 * RPL_ENDOFLINKS
	 *
	 * "<mask> :End of /LINKS list"
	 *
	 * In replying to the LINKS message, a server must send
	 * replies back using the RPL_LINKS numeric and mark the
	 * end of the list using an RPL_ENDOFLINKS reply.
	 */
	endoflinks = 365,

	/**
	 * RPL_BANLIST
	 *
	 * "<channel> <banid>"
	 */
	banlist = 367,

	/**
	 * RPL_ENDOFBANLIST
	 *
	 * "<channel> :End of channel ban list"
	 *
	 * When listing the active 'bans' for a given channel,
	 * a server is required to send the list back using the
	 * RPL_BANLIST and RPL_ENDOFBANLIST messages.  A separate
	 * RPL_BANLIST is sent for each active banid.  After the
	 * banids have been listed (or if none present) a
	 * RPL_ENDOFBANLIST must be sent.
	 */
	endofbanlist = 368,

	/**
	 * RPL_INFO
	 *
	 * ":<string>"
	 */
	info = 371,

	/**
	 * RPL_ENDOFINFO
	 *
	 * ":End of /INFO list"
	 *
	 * A server responding to an INFO message is required to
	 * send all its 'info' in a series of RPL_INFO messages
	 * with a RPL_ENDOFINFO reply to indicate the end of the
	 * replies.
	 */
	endofinfo = 374,

	/**
	 * RPL_MOTDSTART
	 *
	 * ":- <server> Message of the day - "
	 */
	motdstart = 375,

	/**
	 * RPL_MOTD
	 *
	 * ":- <text>"
	 */
	motd = 372,

	/**
	 * RPL_ENDOFMOTD
	 *
	 * ":End of /MOTD command"
	 *
	 * When responding to the MOTD message and the MOTD file
	 * is found, the file is displayed line by line, with
	 * each line no longer than 80 characters, using
	 * RPL_MOTD format replies.  These should be surrounded
	 * by a RPL_MOTDSTART (before the RPL_MOTDs) and an
	 * RPL_ENDOFMOTD (after).
	 */
	endofmotd = 376,

	/**
	 * RPL_YOUREOPER
	 *
	 * ":You are now an IRC operator"
	 *
	 * RPL_YOUREOPER is sent back to a client which has
	 * just successfully issued an OPER message and gained
	 * operator status.
	 */
	youreoper = 381,

	/**
	 * RPL_REHASHING
	 *
	 * "<config file> :Rehashing"
	 *
	 * If the REHASH option is used and an operator sends
	 * a REHASH message, an RPL_REHASHING is sent back to
	 * the operator.
	 */
	rehashing = 382,

	/**
	 * RPL_TIME
	 *
	 * "<server> :<string showing server's local time>"
	 *
	 * When replying to the TIME message, a server must send
	 * the reply using the RPL_TIME format above.  The string
	 * showing the time need only contain the correct day and
	 * time there.  There is no further requirement for the
	 * time string.
	 */
	time = 391,

	/**
	 * RPL_USERSSTART
	 *
	 * ":UserID   Terminal  Host"
	 */
	userstart = 392,

	/**
	 * RPL_USERS
	 *
	 * ":%-8s %-9s %-8s"
	 */
	users = 393,

	/**
	 * RPL_ENDOFUSERS
	 *
	 * ":End of users"
	 */
	endofusers = 394,

	/**
	 * RPL_NOUSERS
	 *
	 * ":Nobody logged in"
	 *
	 * If the USERS message is handled by a server, the
	 * replies RPL_USERSTART, RPL_USERS, RPL_ENDOFUSERS and
	 * RPL_NOUSERS are used.  RPL_USERSSTART must be sent
	 * first, following by either a sequence of RPL_USERS
	 * or a single RPL_NOUSER.  Following this is
	 * RPL_ENDOFUSERS.
	 */
	nousers = 395,

	/**
	 * RPL_TRACELINK
	 *
	 * "Link <version & debug level> <destination> <next server>"
	 */
	tracelink = 200,

	/**
	 * RPL_TRACECONNECTING
	 *
	 * "Try. <class> <server>"
	 */
	traceconnecting = 201,

	/**
	 * RPL_TRACEHANDSHAKE
	 *
	 * "H.S. <class> <server>"
	 */
	tracehandshake = 202,

	/**
	 * RPL_TRACEUNKNOWN
	 *
	 * "???? <class> [<client IP address in dot form>]"
	 */
	traceunknown = 203,

	/**
	 * RPL_TRACEOPERATOR
	 *
	 * "Oper <class> <nick>"
	 */
	traceoperator = 204,

	/**
	 * RPL_TRACEUSER
	 *
	 * "User <class> <nick>"
	 */
	traceuser = 205,

	/**
	 * RPL_TRACESERVER
	 *
	 * "Serv <class> <int>S <int>C <server> \
	 *  <nick!user|*!*>@<host|server>
	 */
	traceserver = 206,

	/**
	 * RPL_TRACENEWTYPE
	 *
	 * "<newtype> 0 <client name>"
	 */
	tracenewtype = 208,

	/**
	 * RPL_TRACELOG
	 *
	 * "File <logfile> <debug level>"
	 *
	 * The RPL_TRACE* are all returned by the server in
	 * response to the TRACE message.  How many are
	 * returned is dependent on the the TRACE message and
	 * whether it was sent by an operator or not.  There
	 * is no predefined order for which occurs first.
	 * Replies RPL_TRACEUNKNOWN, RPL_TRACECONNECTING and
	 * RPL_TRACEHANDSHAKE are all used for connections
	 * which have not been fully established and are either
	 * unknown, still attempting to connect or in the
	 * process of completing the 'server handshake'.
	 * RPL_TRACELINK is sent by any server which handles
	 * a TRACE message and has to pass it on to another
	 * server.  The list of RPL_TRACELINKs sent in
	 * response to a TRACE command traversing the IRC
	 * network should reflect the actual connectivity of
	 * the servers themselves along that path.
	 * RPL_TRACENEWTYPE is to be used for any connection
	 * which does not fit in the other categories but is
	 * being displayed anyway.
	 */
	tracelog = 261,

	/**
	 * RPL_STATSLINKINFO
	 *
	 * "<linkname> <sendq> <sent messages> \
	 *  <sent bytes> <received messages> \
	 *  <received bytes> <time open>"
	 */
	statslinkinfo = 211,

	/**
	 * RPL_STATSCOMMANDS
	 *
	 * "<command> <count>"
	 */
	statscommands = 212,

	/**
	 * RPL_STATSCLINE
	 *
	 * "C <host> * <name> <port> <class>"
	 */
	statscline = 213,

	/**
	 * RPL_STATSNLINE
	 *
	 * "N <host> * <name> <port> <class>"
	 */
	statsnline = 214,

	/**
	 * RPL_STATSILINE
	 *
	 * "I <host> * <host> <port> <class>"
	 */
	statsiline = 215,

	/**
	 * RPL_STATSKLINE
	 *
	 * K <host> * <username> <port> <class>"
	 */
	statskline = 216,

	/**
	 * RPL_STATSYLINE
	 *
	 * "Y <class> <ping frequency> <connect frequency> <max sendq>"
	 */
	statsyline = 218,

	/**
	 * RPL_ENDOFSTATS
	 *
	 * "<stats letter> :End of /STATS report"
	 */
	endofstats = 219,

	/**
	 * RPL_STATSLLINE
	 *
	 * "L <hostmask> * <servername> <maxdepth>"
	 */
	statslline = 241,

	/**
	 * RPL_STATSUPTIME
	 *
	 * ":Server Up %d days %d:%02d:%02d"
	 */
	statsuptime = 242,

	/**
	 * RPL_STATSOLINE
	 *
	 * "O <hostmask> * <name>"
	 */
	statsoline = 243,

	/**
	 * RPL_STATSHLINE
	 *
	 * "H <hostmask> * <servername>"
	 */
	statshline = 244,

	/**
	 * RPL_UMODEIS
	 *
	 * "<user mode string>"
	 *
	 * To answer a query about a client's own mode,
	 * RPL_UMODEIS is sent back.
	 */
	umodeis = 221,

	/**
	 * RPL_LUSERCLIENT
	 *
	 * ":There are <integer> users and <integer> \
	 *  invisible on <integer> servers"
	 */
	luserclient = 251,

	/**
	 * RPL_LUSEROP
	 *
	 * "<integer> :operator(s) online"
	 */
	luserop = 252,

	/**
	 * RPL_LUSERUNKNOWN
	 *
	 * "<integer> :unknown connection(s)"
	 */
	luserunknown = 253,

	/**
	 * RPL_LUSERCHANNELS
	 *
	 * "<integer> :channels formed"
	 */
	luserchannels = 254,

	/**
	 * RPL_LUSERME
	 *
	 * ":I have <integer> clients and <integer> servers"
	 *
	 * In processing an LUSERS message, the server
	 * sends a set of replies from RPL_LUSERCLIENT,
	 * RPL_LUSEROP, RPL_USERUNKNOWN,
	 * RPL_LUSERCHANNELS and RPL_LUSERME.  When
	 * replying, a server must send back
	 * RPL_LUSERCLIENT and RPL_LUSERME.  The other
	 * replies are only sent back if a non-zero count
	 * is found for them.
	 */
	luserme = 255,

	/**
	 * RPL_ADMINME
	 *
	 * "<server> :Administrative info"
	 */
	adminme = 256,

	/**
	 * RPL_ADMINLOC1
	 *
	 * ":<admin info>"
	 */
	adminloc1 = 257,

	/**
	 * RPL_ADMINLOC2
	 *
	 * ":<admin info>"
	 */
	adminloc2 = 258,

	/**
	 * RPL_ADMINEMAIL
	 *
	 * ":<admin info>"
	 *
	 * When replying to an ADMIN message, a server
	 * is expected to use replies RLP_ADMINME
	 * through to RPL_ADMINEMAIL and provide a text
	 * message with each.  For RPL_ADMINLOC1 a
	 * description of what city, state and country
	 * the server is in is expected, followed by
	 * details of the university and department
	 * (RPL_ADMINLOC2) and finally the administrative
	 * contact for the server (an email address here
	 * is required) in RPL_ADMINEMAIL.
	 */
	adminemail = 259
};

/**
 * \brief Describe a IRC message
 */
struct message {
	std::string prefix;             //!< optional prefix
	std::string command;            //!< command (maybe string or code)
	std::vector<std::string> args;  //!< parameters

	/**
	 * Check if the command is of the given enum number.
	 *
	 * \param e the code
	 * \return true if command is a number and equals to e
	 */
	template <typename Enum>
	auto is(Enum e) const noexcept -> bool
	{
		try {
			return std::stoi(command) == static_cast<int>(e);
		} catch (...) {
			return false;
		}
	}

	/**
	 * Convenient function that returns an empty string if the nth argument is
	 * not defined.
	 *
	 * \param index the index
	 * \return a string or empty if out of bounds
	 */
	auto get(unsigned short index) const noexcept -> const std::string&;

	/**
	 * Tells if the message is a CTCP.
	 *
	 * \param index the param index (maybe out of bounds)
	 * \return true if CTCP
	 */
	auto is_ctcp(unsigned short index) const noexcept -> bool;

	/**
	 * Parse a CTCP message.
	 *
	 * \pre is_ctcp(index)
	 * \param index the param index
	 * \return the CTCP command
	 */
	auto ctcp(unsigned short index) const -> std::string;

	/**
	 * Parse a IRC message.
	 *
	 * \param line the buffer content (without `\r\n`)
	 * \return the message (maybe empty if line is empty)
	 */
	static auto parse(const std::string& line) -> message;
};

/**
 * \brief Describe a user.
 */
struct user {
	std::string nick;       //!< The nickname
	std::string host;       //!< The hostname

	/**
	 * Parse a nick/host combination.
	 *
	 * \param line the line to parse
	 * \return a user
	 */
	static auto parse(std::string_view line) -> user;
};

/**
 * \brief Abstract connection to a server.
 */
class connection {
public:
	/**
	 * Handler for connecting.
	 */
	using connect_handler = std::function<void (std::error_code)>;

	/**
	 * Handler for receiving.
	 */
	using recv_handler = std::function<void (std::error_code, message)>;

	/**
	 * Handler for sending.
	 */
	using send_handler = std::function<void (std::error_code)>;

private:
	boost::asio::io_context& service_;
	boost::asio::ip::tcp::socket socket_{service_};
	boost::asio::ip::tcp::resolver resolver_{service_};
	boost::asio::streambuf input_{1024};
	boost::asio::streambuf output_;

	bool ipv4_{true};
	bool ipv6_{true};
	bool ssl_{false};

#if defined(IRCCD_HAVE_SSL)
	boost::asio::ssl::context context_{boost::asio::ssl::context::tlsv12};
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket&> ssl_socket_{socket_, context_};
#endif

#if !defined(NDEBUG)
	bool is_connecting_{false};
	bool is_receiving_{false};
	bool is_sending_{false};
#endif

	void handshake(const connect_handler&);
	void connect(const boost::asio::ip::tcp::resolver::results_type&, const connect_handler&);
	void resolve(std::string_view, std::string_view, const connect_handler&);

public:
	/**
	 * Default constructor.
	 *
	 * \param service the I/O service
	 */
	connection(boost::asio::io_service& service);

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~connection() = default;

	/**
	 * Enable IPv4
	 *
	 * \param enable true to enable
	 */
	void use_ipv4(bool enable = true) noexcept;

	/**
	 * Enable IPv6
	 *
	 * \param enable true to enable
	 */
	void use_ipv6(bool enable = true) noexcept;

	/**
	 * Enable TLS.
	 *
	 * \pre IRCCD_HAVE_SSL must be defined
	 * \param enable true to enable
	 */
	void use_ssl(bool enable = true) noexcept;

	/**
	 * Connect to the host.
	 *
	 * \pre handler the handler
	 * \pre another connect operation must not be running
	 * \pre ipv4 or ipv6 must be set
	 * \param hostname the hostname
	 * \param service the service or port number
	 * \param handler the non-null handler
	 */
	void connect(std::string_view hostname, std::string_view service, connect_handler handler);

	/**
	 * Force disconnection.
	 */
	void disconnect();

	/**
	 * Start receiving data.
	 *
	 * The handler must not throw exceptions and `this` must be valid in the
	 * lifetime of the handler.
	 *
	 * \pre another recv operation must not be running
	 * \pre handler != nullptr
	 * \param handler the handler to call
	 */
	void recv(recv_handler handler);

	/**
	 * Start sending data.
	 *
	 * The handler must not throw exceptions and `this` must be valid in the
	 * lifetime of the handler.
	 *
	 * \pre another send operation must not be running
	 * \pre handler != nullptr
	 * \param message the raw message
	 * \param handler the handler to call
	 */
	void send(std::string_view message, send_handler handler);
};

} // !irccd::daemon::irc

#endif // !IRCCD_IRC_HPP
