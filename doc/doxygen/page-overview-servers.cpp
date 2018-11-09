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
