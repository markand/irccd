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
