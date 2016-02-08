/*
 * transport-client.cpp -- client connected to irccd
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

#include <json.h>
#include <logger.h>

#include "transport-client.h"

namespace irccd {

void TransportClient::parse(const std::string &message)
{
	try {
		json::Value document(json::Buffer{message});

		if (!document.isObject())
			throw std::invalid_argument("the message is not a valid JSON object");

		onCommand(document);
	} catch (const std::exception &ex) {
		// TODO: send general error to the client
		log::warning() << "transport: " << ex.what() << std::endl;
	}
}

void TransportClient::sync(fd_set &setinput, fd_set &setoutput)
{
	if (FD_ISSET(handle(), &setinput)) {
		log::debug() << "transport: receiving to input buffer" << std::endl;
		receive();
	}
	if (FD_ISSET(handle(), &setoutput)) {
		log::debug() << "transport: sending outgoing buffer" << std::endl;
		send();
	}
}

void TransportClient::ok(const std::string &command)
{
	m_output += "{";
	m_output += "\"response\":\"" + command + "\",";
	m_output += "\"status\":\"ok\"";
	m_output += "}\r\n\r\n";
}

void TransportClient::error(const std::string &command, std::string message)
{
	m_output += "{";
	m_output += "\"response\":\"" + command + "\",";
	m_output += "\"status\":\"error\",";
	m_output += "\"error\": \"" + json::escape(message) + "\"";
	m_output += "}\r\n\r\n";
}

void TransportClient::send(std::string message)
{
	m_output += message;
	m_output += "\r\n\r\n";
}

} // !irccd
