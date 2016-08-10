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

#include "json.hpp"
#include "logger.hpp"
#include "transport-client.hpp"

namespace irccd {

void TransportClient::parse(const std::string &message)
{
    auto document = nlohmann::json::parse(message);

    if (document.is_object())
        onCommand(document);
}

void TransportClient::receive()
{
    try {
        std::string buffer;

        buffer.resize(512);
        buffer.resize(m_socket.recv(&buffer[0], buffer.size()));

        if (buffer.empty())
            onDie();

        m_input += std::move(buffer);
    } catch (const std::exception &) {
        onDie();
    }
}

void TransportClient::send()
{
    try {
        auto ns = m_socket.send(&m_output[0], m_output.size());

        if (ns == 0)
            onDie();

        m_output.erase(0, ns);
    } catch (const std::exception &ex) {
        onDie();
    }
}

void TransportClient::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    if (m_socket.handle() > max)
        max = m_socket.handle();

    FD_SET(m_socket.handle(), &in);

    if (!m_output.empty())
        FD_SET(m_socket.handle(), &out);
}

void TransportClient::sync(fd_set &in, fd_set &out)
{
    // Do some I/O.
    if (FD_ISSET(m_socket.handle(), &in)) {
        log::debug() << "transport: receiving to input buffer" << std::endl;
        receive();
    }
    if (FD_ISSET(m_socket.handle(), &out)) {
        log::debug() << "transport: sending outgoing buffer" << std::endl;
        send();
    }

    // Flush the queue.
    for (std::size_t pos; (pos = m_input.find("\r\n\r\n")) != std::string::npos; ) {
        auto message = m_input.substr(0, pos);

        m_input.erase(m_input.begin(), m_input.begin() + pos + 4);

        parse(message);
    }
}

void TransportClient::send(const nlohmann::json &json)
{
    assert(json.is_object());

    m_output += json.dump();
    m_output += "\r\n\r\n";
}

} // !irccd
