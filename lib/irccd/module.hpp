#ifndef IRCCD_MODULE_HPP
#define IRCCD_MODULE_HPP

#include <memory>

namespace irccd {

class Irccd;
class JsPlugin;

class Module {
public:
	/**
	 * Default constructor.
	 */
	Module() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~Module() = default

	/**
	 * Load the module into the JavaScript plugin.
	 *
	 * \param irccd the irccd instance
	 * \param plugin the plugin
	 */
	void load(Irccd &irccd, const std::shared_ptr<JsPlugin> &plugin);

	/**
	 * Unload the module from the JavaScript plugin.
	 *
	 * \param irccd the irccd instance
	 * \param plugin the plugin
	 */
	void unload(Irccd &irccd, const std::shared_ptr<Plugin> &plugin);
};

} // !irccd

#endif // !IRCCD_MODULE_HPP