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
