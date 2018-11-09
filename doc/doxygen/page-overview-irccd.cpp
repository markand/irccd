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
