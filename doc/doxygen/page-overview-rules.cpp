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
