
/*
 * pages.cpp -- doxygen related pages
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

/**
 * \page overview Overview
 * \brief Architecture overview.
 *
 * \subpage overview-irccd
 */

/**
 * \page overview-irccd Irccd overview
 * \brief Irccd architecture overview.
 *
 * The irccd daemon runs different services to deliver the bot functionalities.
 * The main loop is controlled with a `boost::asio::io_context` and completely
 * mono-threaded.
 *
 * ~~~
 *                      +--------------+
 *                      |              |
 *                      | rule_service |
 *                      |              |
 *                      +-------^------+
 *                              |
 *                              |
 *                              |
 * +----------------+       +---+---+       +-------------------+
 * |                |       |       |       |                   |
 * | plugin_service <-------+ irccd +-------> transport_service |
 * |                |       |       |       |                   |
 * +----------------+       +---+---+       +-------------------+
 *                              |
 *                              |
 *                              |
 *                      +-------v--------+
 *                      |                |
 *                      | server_service |
 *                      |                |
 *                      +----------------+
 * ~~~
 *
 * Look at the different subpages for more information.
 *
 * - \subpage overview-plugins
 * - \subpage overview-rules
 * - \subpage overview-servers
 * - \subpage overview-transports
 */

/**
 * \page overview-plugins Plugins
 * \brief Plugins overview
 *
 * The plugins are the essential part of irccd, they are called for each IRC
 * events for each IRC servers.
 *
 * Plugins can be written in Javascript or in C++.
 *
 * ~~~
 *                    uses                      uses
 * +----------------+       +----------------+         +---------------+
 * |                |       |                |    0..* |               |
 * | server_service +-------> plugin_service +--+------+ plugin_loader |
 * |                |       |                |  |      |               |
 * +----------------+       +----------------+  |      +-------+-------+
 *                                              |              |
 *                                              | invoke       | find or open
 *                                              |              |
 *                                              |          +---v----+
 *                                              |     0..* |        |
 *                                              +----------+ plugin |
 *                                                         |        |
 *                                                         +----^---+
 *                                                              |
 *                                                              | inherits
 *                                                              |
 *                                                        +-----+-----+
 *                                                        |           |
 *                                                        | js_plugin |
 *                                                        |           |
 *                                                        +-----------+
 * ~~~
 *
 * ## The plugin_service
 *
 * The plugin_service class opens, loads, reload or unload plugins. It also
 * invoke plugins for IRC events from server_service.
 *
 * It also uses plugin_loaders objects to find new plugins once requested or
 * opens them.
 *
 * ## The plugin_loader class
 *
 * This abstract class is responsible of searching and opening plugins. It has a
 * convenient predefined function that will search for standard path according
 * to a file extension.
 *
 * ## The plugin class
 *
 * The abstract plugin class is the user point of customization. It contains
 * various virtual functions to be redefined for IRC events the user is
 * interested in.
 *
 * ## The js_plugin class
 *
 * If built with Javascript support and linked against libirccd-js, one can use
 * js_plugin to load Javscript plugin files.
 *
 * This class will call global functions defined in the file. More information
 * in the official Javascript API.
 */

/**
 * \page overview-rules Rules
 * \brief Rules overview
 *
 * The rules is the mechanism in irccd that accept/forbid plugin commands
 * invocations depending on user criterias. It's a kind of plugin firewall.
 *
 * It's usage is pretty simple.
 *
 * ~~~
 *  +----------------+   asks    +--------------+
 *  |                |           |              |
 *  | server_service +-----------> rule_service |
 *  |                |           |              |
 *  +----------------+           +-------+------+
 *                                       |
 *                                       |
 *                                       | 0..*
 *                                    +------+
 *                                    |      |
 *                                    | rule |
 *                                    |      |
 *                                    +------+
 * ~~~
 *
 * ## The rule_service class
 *
 * Owns a set of rule and provide functions to check if a rule will match
 * depending on the following criterias:
 *
 * - the server name
 * - the origin user
 * - the channel name
 * - the plugin name
 * - the event name
 *
 * Then, if the rule match, its action is considered (accept or drop).
 *
 * ## The rule class
 *
 * A simple data that contains all criterias.
 *
 * ## Notes
 *
 * You may wonder why it's server_service that uses rule_service. It's because
 * the server_service is the only one that knows all criterias, some IRC events
 * don't have those.
 *
 * This may change in the future.
 */

/**
 * \page overview-servers Servers
 * \brief Servers overview
 *
 * This page explains the overview related to IRC servers.
 *
 * ~~~
 * +----------------+    invoke    +----------------+
 * |                |              |                |
 * | server_service +------+-------> plugin_service |
 * |                |      |       |                |
 * +-------+--------+      |       +----------------+
 *         |               |
 *         |               | dispatches
 *         | 0..*          |
 *     +---+----+          |       +-------------------+
 *     |        |          |       |                   |
 *     | server |          +-------> transport_service |
 *     |        |                  |                   |
 *     +---+----+                  +-------------------+
 *         |
 *         |
 *         | 1
 *   +-----+------+
 *   |            |
 *   | connection |
 *   |            |
 *   +------------+
 * ~~~
 *
 * ## The server_service class
 *
 * This class is responsible of servers, it receives messages from them and then
 * invoke plugins and dispatches IRC events to all irccdctl clients connected.
 *
 * ## The server class
 *
 * The server class is higher level than connection. It stores all options, user
 * information, settings and messages queues for IRC.
 *
 * It also does authentication and has various IRC commands predefined.
 *
 * ## The connection class
 *
 * The connection class is the lowest part of the IRC connection, it only
 * receives and sends messages with appropriate parsing.
 */

/**
 * \page overview-transports Transports
 * \brief Transports overview
 *
 * The transports feature is dedicated into irccd to irccdctl dialogs. It
 * allows:
 *
 * - Requests from irccdctl,
 * - Events from irccd to all irccdctl,
 * - Different type of protocols (TCP/IP, local and TLS).
 *
 * It consists of different parts.
 *
 * ~~~
 * +-------------------+         +---------+
 * |                   |    0..* |         |
 * | transport_service +---------+ command |
 * |                   |         |         |
 * +---------+---------+         +----^----+
 *           |                        |
 *           |                        | executes
 *           | 0..*                   |
 * +---------+--------+          +----+-------------+
 * |                  | 1   0..* |                  |
 * | transport_server +----------+ transport_client |
 * |                  |          |                  |
 * +------------------+          +------------------+
 * ~~~
 *
 * ## The transport_service class
 *
 * This class owns several transport_server, it will wait for a new client in
 * each of those servers.
 *
 * It's also dedicated to broadcast messages to all connected transport_clients.
 *
 * ## The transport_server class
 *
 * This class has only one purpose, to accept a new client. It's abstract and
 * the underlying implementation is responsible of doing its own operation.
 *
 * ## The transport_client class
 *
 * This stateful class represent a direct connection to a irccdctl client.
 *
 * It does authentication if required and process input messages. It does not
 * call commands directly but dispatch that to the transport_service.
 *
 * ## The command class
 *
 * This abstract class defines an operation to perform.
 */
