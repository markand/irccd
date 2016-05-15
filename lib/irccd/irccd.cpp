/*
 * irccd.cpp -- main irccd class
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

#include "irccd.hpp"
#include "logger.hpp"
#include "service-interrupt.hpp"
#include "service-module.hpp"
#include "service-plugin.hpp"
#include "service-rule.hpp"
#include "service-server.hpp"
#include "service-transport.hpp"
#include "sockets.hpp"

using namespace std;
using namespace std::placeholders;
using namespace std::string_literals;

namespace irccd {

Irccd::Irccd()
	: m_interruptService(std::make_shared<InterruptService>())
	, m_serverService(std::make_shared<ServerService>(*this))
	, m_transportService(std::make_shared<TransportService>(*this))
	, m_ruleService(std::make_shared<RuleService>())
	, m_moduleService(std::make_shared<ModuleService>())
	, m_pluginService(std::make_shared<PluginService>(*this))
{
	m_services.push_back(m_interruptService);
	m_services.push_back(m_serverService);
	m_services.push_back(m_transportService);
}

void Irccd::post(std::function<void (Irccd &)> ev) noexcept
{
	std::lock_guard<mutex> lock(m_mutex);

	m_events.push_back(move(ev));
	m_interruptService->interrupt();
}

void Irccd::run()
{
	while (m_running) {
		poll();
		dispatch();
	}
}

void Irccd::poll()
{
	fd_set setinput;
	fd_set setoutput;
	net::Handle max = 0;

	FD_ZERO(&setinput);
	FD_ZERO(&setoutput);

	for (const auto &service : m_services) {
		service->prepare(setinput, setoutput, max);
	}

	// Do the selection.
	struct timeval tv;

	tv.tv_sec = 5;
	tv.tv_usec = 250000;

	int error = select(max + 1, &setinput, &setoutput, nullptr, &tv);

	// Skip anyway if requested to stop
	if (!m_running) {
		return;
	}

	// Skip on error.
	if (error < 0 && errno != EINTR) {
		log::warning() << "irccd: " << net::error(error) << endl;
		return;
	}

	// Process after selection.
	for (const auto &service : m_services) {
		service->sync(setinput, setoutput);
	}
}

void Irccd::dispatch()
{
	/*
	 * Make a copy because the events can add other events while we are iterating it. Also lock because the timers
	 * may alter these events too.
	 */
	std::vector<std::function<void (Irccd &)>> copy;

	{
		std::lock_guard<mutex> lock(m_mutex);

		copy = move(m_events);
		m_events.clear();
	}

	if (copy.size() > 0) {
		log::debug() << "irccd: dispatching " << copy.size() << " event" << (copy.size() > 1 ? "s" : "") << endl;
	}

	for (auto &ev : copy) {
		ev(*this);
	}
}

void Irccd::stop()
{
	log::debug() << "irccd: requesting to stop now" << endl;

	m_running = false;
	m_interruptService->interrupt();
}

} // !irccd
