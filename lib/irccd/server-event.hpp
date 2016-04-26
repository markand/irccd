/*
 * server-event.hpp -- server event
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_SERVER_EVENT_HPP
#define IRCCD_SERVER_EVENT_HPP

/**
 * \file server-event.hpp
 * \brief IRC events.
 */

#include <functional>
#include <string>

#include <sysconfig.hpp>

#if defined(WITH_JS)

namespace irccd {

class Irccd;
class Plugin;

/**
 * \brief Dispatch IRC event to plugins.
 *
 * This event will iterate all plugins and check if no rules blocks this event, otherwise the plugin function
 * will be called.
 */
class ServerEvent {
private:
	std::string m_server;
	std::string m_origin;
	std::string m_target;
	std::function<std::string (Plugin &)> m_plugin_function_name;
	std::function<void (Plugin &)> m_plugin_exec;

public:
	/**
	 * Constructor.
	 *
	 * \param server the server name
	 * \param origin the origin
	 * \param target the target (channel or nickname)
	 * \param plugin_function_name the JavaScript function to call (only for rules)
	 * \param plugin_exec the plugin executor
	 */
	ServerEvent(std::string server,
		    std::string origin,
		    std::string target,
		    std::function<std::string (Plugin &)> plugin_function_name,
		    std::function<void (Plugin &)> plugin_exec);

	/**
	 * Execute the event.
	 *
	 * \param irccd the irccd instance
	 */
	void operator()(Irccd &irccd) const;
};

} // !irccd

#endif // !WITH_JS

#endif // !IRCCD_SERVER_EVENT_HPP
