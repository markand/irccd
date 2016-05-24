/*
 * service-transport.cpp -- manage transport servers and clients
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
#include "service-transport.hpp"
#include "transport-client.hpp"
#include "transport-server.hpp"

namespace irccd {

void TransportService::handleCommand(std::weak_ptr<TransportClient> ptr, const json::Value &object)
{
	assert(object.isObject());

	m_irccd.post([=] (Irccd &) {
		// 0. Be sure the object still exists.
		auto tc = ptr.lock();

		if (!tc)
			return;

		// 1. Check if the Json object is valid.
		auto name = object.find("command");
		if (name == object.end() || name->typeOf() != json::Type::String) {
			// TODO: send error
			log::warning("invalid command object");
			return;
		}

		// 2. Search for a command
		auto it = m_irccd.commands().find(name->toString());

		if (it == m_irccd.commands().end()) {
			// TODO: send error again
			log::warning("command does not exists");
			return;
		}

		// 3. Try to execute it.
		json::Value response = json::object({});

		try {
			response = it->second->exec(m_irccd, object);

			// Adjust if command has returned something else.
			if (!response.isObject()) {
				response = json::object({});
			}

			response.insert("status", true);
		} catch (const std::exception &ex) {
			response.insert("status", false);
			response.insert("error", ex.what());
		}

		// 4. Store the command name result.
		response.insert("response", it->first);

		// 5. Send the result.
		tc->send(response.toJson(0));
	});
}

void TransportService::handleDie(std::weak_ptr<TransportClient> ptr)
{
	m_irccd.post([=] (Irccd &) {
		log::info("transport: client disconnected");

		auto tc = ptr.lock();

		if (tc)
			m_clients.erase(std::find(m_clients.begin(), m_clients.end(), tc));
	});
}

TransportService::TransportService(Irccd &irccd) noexcept
	: m_irccd(irccd)
{
}

void TransportService::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
	// Add transport servers.
	for (const auto &transport : m_servers) {
		FD_SET(transport->handle(), &in);

		if (transport->handle() > max)
			max = transport->handle();
	}

	// Transport clients.
	for (const auto &client : m_clients) {
		FD_SET(client->handle(), &in);

		if (client->hasOutput())
			FD_SET(client->handle(), &out);
		if (client->handle() > max)
			max = client->handle();
	}
}

void TransportService::sync(fd_set &in, fd_set &out)
{
	using namespace std::placeholders;

	// Transport servers.
	for (const auto &transport : m_servers) {
		if (!FD_ISSET(transport->handle(), &in))
			continue;

		log::debug("transport: new client connected");

		std::shared_ptr<TransportClient> client = transport->accept();
		std::weak_ptr<TransportClient> ptr(client);

		// Send some information.
		json::Value object = json::object({
			{ "program",	"irccd"			},
			{ "major",	IRCCD_VERSION_MAJOR	},
			{ "minor",	IRCCD_VERSION_MINOR	},
			{ "patch",	IRCCD_VERSION_PATCH	}
		});

#if defined(WITH_JS)
		object.insert("javascript", true);
#endif
#if defined(WITH_SSL)
		object.insert("ssl", true);
#endif

		client->send(object.toJson(0));

		// Connect signals.
		client->onCommand.connect(std::bind(&TransportService::handleCommand, this, ptr, _1));
		client->onDie.connect(std::bind(&TransportService::handleDie, this, ptr));

		// Register it.
		m_clients.push_back(std::move(client));
	}

	// Transport clients.
	for (const auto &client : m_clients) {
		client->sync(in, out);
	}
}

void TransportService::add(std::shared_ptr<TransportServer> ts)
{
	m_servers.push_back(std::move(ts));
}

void TransportService::broadcast(std::string data)
{
	// Asynchronous send.
	for (const auto &client : m_clients)
		client->send(data);
}

} // !irccd
