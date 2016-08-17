/*
 * connection.cpp -- value wrapper for connecting to irccd
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

#include <stdexcept>

#include <format.h>

#include "connection.hpp"
#include "util.hpp"

using namespace fmt::literals;

namespace irccd {

/*
 * Connection::State.
 * ------------------------------------------------------------------
 */

class Connection::State {
public:
    State() = default;
    virtual ~State() = default;
    virtual Status status() const noexcept = 0;
    virtual void prepare(Connection &cnt, fd_set &in, fd_set &out) = 0;
    virtual void sync(Connection &cnt, fd_set &in, fd_set &out) = 0;
};

/*
 * Connection::DisconnectedState.
 * ------------------------------------------------------------------
 */

class Connection::DisconnectedState : public Connection::State {
public:
    Connection::Status status() const noexcept override
    {
        return Disconnected;
    }

    void prepare(Connection &, fd_set &, fd_set &) override {}
    void sync(Connection &, fd_set &, fd_set &) override {}
};

/*
 * Connection::DisconnectedState.
 * ------------------------------------------------------------------
 */

class Connection::ReadyState : public Connection::State {
private:
    void parse(Connection &cnx, const std::string &message)
    {
        try {
            auto json = nlohmann::json::parse(message);

            if (!json.is_object())
                return;

            cnx.onMessage(json);
        } catch (const std::exception &) {
        }
    }
public:
    Connection::Status status() const noexcept override
    {
        return Ready;
    }

    void prepare(Connection &cnx, fd_set &in, fd_set &out) override
    {
        FD_SET(cnx.m_socket.handle(), &in);

        if (!cnx.m_output.empty())
            FD_SET(cnx.m_socket.handle(), &out);
    }

    void sync(Connection &cnx, fd_set &in, fd_set &out) override
    {
        if (FD_ISSET(cnx.m_socket.handle(), &out))
            cnx.send();

        if (FD_ISSET(cnx.m_socket.handle(), &in))
            cnx.recv();

        std::string msg;

        do {
            msg = util::nextNetwork(cnx.m_input);

            if (!msg.empty())
                parse(cnx, msg);
        } while (!msg.empty());
    }
};

/*
 * Connection::AuthState.
 * ------------------------------------------------------------------
 */

class Connection::AuthState : public Connection::State {
private:
    enum {
        Created,
        Sending,
        Checking
    } m_auth{Created};

    std::string m_output;

    void send(Connection &cnt) noexcept
    {
        try {
            auto n = cnt.send(m_output.data(), m_output.size());

            if (n == 0) {
                m_output.clear();
                throw std::runtime_error("connection lost");
            }

            m_output.erase(0, n);

            if (m_output.empty())
                m_auth = Checking;
        } catch (const std::exception &ex) {
            cnt.m_state = std::make_unique<DisconnectedState>();
            cnt.onDisconnect(ex.what());
        }
    }

    void check(Connection &cnt) noexcept
    {
        cnt.recv();

        auto msg = util::nextNetwork(cnt.m_input);

        if (msg.empty())
            return;

        try {
            auto doc = nlohmann::json::parse(msg);

            if (!doc.is_object())
                throw std::invalid_argument("invalid argument");

            auto cmd = doc.find("response");

            if (cmd == doc.end() || !cmd->is_string() || *cmd != "auth")
                throw std::invalid_argument("authentication result expected");

            auto result = doc.find("result");

            if (result == doc.end() || !result->is_boolean())
                throw std::invalid_argument("bad protocol");

            if (!*result)
                throw std::runtime_error("authentication failed");

            cnt.m_state = std::make_unique<ReadyState>();
        } catch (const std::exception &ex) {
            cnt.m_state = std::make_unique<DisconnectedState>();
            cnt.onDisconnect(ex.what());
        }
    }

public:
    Connection::Status status() const noexcept override
    {
        return Authenticating;
    }

    void prepare(Connection &cnt, fd_set &in, fd_set &out) override
    {
        switch (m_auth) {
        case Created:
            m_auth = Sending;
            m_output += nlohmann::json({
                { "command", "auth" },
                { "password", cnt.m_password }
            }).dump();
            m_output += "\r\n\r\n";

            // FALLTHROUGH
        case Sending:
            FD_SET(cnt.m_socket.handle(), &out);
            break;
        case Checking:
            FD_SET(cnt.m_socket.handle(), &in);
            break;
        default:
            break;
        }
    }

    void sync(Connection &cnt, fd_set &in, fd_set &out) override
    {
        switch (m_auth) {
        case Sending:
            if (FD_ISSET(cnt.m_socket.handle(), &out))
                send(cnt);
            break;
        case Checking:
            if (FD_ISSET(cnt.m_socket.handle(), &in))
                check(cnt);
            break;
        default:
            break;
        }
    }
};

/*
 * Connection::CheckingState.
 * ------------------------------------------------------------------
 */

class Connection::CheckingState : public Connection::State {
private:
    void verifyProgram(const nlohmann::json &json) const
    {
        auto prog = json.find("program");

        if (prog == json.end() || !prog->is_string() || prog->get<std::string>() != "irccd")
            throw std::runtime_error("not an irccd instance");
    }

    void verifyVersion(Connection &cnx, const nlohmann::json &json) const
    {
        auto getVersionVar = [&] (auto key) {
            auto it = json.find(key);

            if (it == json.end() || !it->is_number_unsigned())
                throw std::runtime_error("invalid irccd instance");

            return *it;
        };

        Info info{
            getVersionVar("major"),
            getVersionVar("minor"),
            getVersionVar("patch")
        };

        // Ensure compatibility.
        if (info.major != IRCCD_VERSION_MAJOR || info.minor > IRCCD_VERSION_MINOR)
            throw std::runtime_error("server version too recent {}.{}.{} vs {}.{}.{}"_format(
                info.major, info.minor, info.patch,
                IRCCD_VERSION_MAJOR, IRCCD_VERSION_MINOR, IRCCD_VERSION_PATCH));

        // Successfully connected.
        if (cnx.m_password.empty())
            cnx.m_stateNext = std::make_unique<ReadyState>();
        else
            cnx.m_stateNext = std::make_unique<AuthState>();

        cnx.onConnect(info);
    }

    void verify(Connection &cnx) const
    {
        auto msg = util::nextNetwork(cnx.m_input);

        if (msg.empty())
            return;

        try {
            auto json = nlohmann::json::parse(msg);

            verifyProgram(json);
            verifyVersion(cnx, json);
        } catch (const std::exception &ex) {
            cnx.m_stateNext = std::make_unique<DisconnectedState>();
            cnx.onDisconnect(ex.what());
        }
    }

public:
    Connection::Status status() const noexcept override
    {
        return Checking;
    }

    void prepare(Connection &cnx, fd_set &in, fd_set &) override
    {
        FD_SET(cnx.m_socket.handle(), &in);
    }

    void sync(Connection &cnx, fd_set &, fd_set &) override
    {
        cnx.recv();

        verify(cnx);
    }
};

/*
 * Connection::ConnectingState.
 * ------------------------------------------------------------------
 */

class Connection::ConnectingState : public Connection::State {
public:
    Connection::Status status() const noexcept override
    {
        return Connecting;
    }

    void prepare(Connection &cnx, fd_set &, fd_set &out) override
    {
        FD_SET(cnx.m_socket.handle(), &out);
    }

    void sync(Connection &cnx, fd_set &, fd_set &out) override
    {
        if (!FD_ISSET(cnx.m_socket.handle(), &out))
            return;

        try {
            auto errc = cnx.m_socket.get<int>(SOL_SOCKET, SO_ERROR);

            if (errc != 0) {
                cnx.m_stateNext = std::make_unique<DisconnectedState>();
                cnx.onDisconnect(net::error(errc));
            } else
                cnx.m_stateNext = std::make_unique<CheckingState>();
        } catch (const std::exception &ex) {
            cnx.m_stateNext = std::make_unique<DisconnectedState>();
            cnx.onDisconnect(ex.what());
        }
    }
};

/*
 * Connection.
 * ------------------------------------------------------------------
 */

unsigned Connection::recv(char *buffer, unsigned length)
{
    return m_socket.recv(buffer, length);
}

unsigned Connection::send(const char *buffer, unsigned length)
{
    return m_socket.send(buffer, length);
}

void Connection::recv()
{
    try {
        std::string buffer;

        buffer.resize(512);
        buffer.resize(recv(&buffer[0], buffer.size()));

        if (buffer.empty())
            throw std::runtime_error("connection lost");

        m_input += std::move(buffer);
    } catch (const std::exception &ex) {
        m_stateNext = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

void Connection::send()
{
    try {
        auto ns = send(m_output.data(), m_output.length());

        if (ns > 0)
            m_output.erase(0, ns);
    } catch (const std::exception &ex) {
        m_stateNext = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

Connection::Connection()
    : m_state(std::make_unique<DisconnectedState>())
{
}

Connection::~Connection() = default;

Connection::Status Connection::status() const noexcept
{
    return m_state->status();
}

void Connection::connect(const net::Address &address)
{
    assert(status() == Disconnected);

    try {
        m_socket = net::TcpSocket(address.domain(), 0);
        m_socket.set(net::option::SockBlockMode(false));
        m_socket.connect(address);
        m_state = std::make_unique<CheckingState>();
    } catch (const net::WouldBlockError &) {
        m_state = std::make_unique<ConnectingState>();
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

void Connection::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    try {
        m_state->prepare(*this, in, out);

        if (m_socket.handle() > max)
            max = m_socket.handle();
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

void Connection::sync(fd_set &in, fd_set &out)
{
    try {
        m_state->sync(*this, in, out);

        if (m_stateNext) {
            m_state = std::move(m_stateNext);
            m_stateNext = nullptr;
        }
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

/*
 * TlsConnection.
 * ------------------------------------------------------------------
 */

void TlsConnection::handshake()
{
    try {
        m_ssl->handshake();
        m_handshake = HandshakeReady;
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    } catch (const std::exception &ex) {
        m_state = std::make_unique<DisconnectedState>();
        onDisconnect(ex.what());
    }
}

unsigned TlsConnection::recv(char *buffer, unsigned length)
{
    unsigned nread = 0;

    try {
        nread = m_ssl->recv(buffer, length);
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    }

    return nread;
}

unsigned TlsConnection::send(const char *buffer, unsigned length)
{
    unsigned nsent = 0;

    try {
        nsent = m_ssl->send(buffer, length);
    } catch (const net::WantReadError &) {
        m_handshake = HandshakeRead;
    } catch (const net::WantWriteError &) {
        m_handshake = HandshakeWrite;
    }

    return nsent;
}

void TlsConnection::connect(const net::Address &address)
{
    Connection::connect(address);

    m_ssl = std::make_unique<net::TlsSocket>(m_socket, net::TlsSocket::Client);
}

void TlsConnection::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    if (m_state->status() == Connecting)
        Connection::prepare(in, out, max);
    else {
        if (m_socket.handle() > max)
            max = m_socket.handle();

        /*
         * Attempt an immediate handshake immediately if connection succeeded
         * in last iteration.
         */
        if (m_handshake == HandshakeUndone)
            handshake();

        switch (m_handshake) {
        case HandshakeRead:
            FD_SET(m_socket.handle(), &in);
            break;
        case HandshakeWrite:
            FD_SET(m_socket.handle(), &out);
            break;
        default:
            Connection::prepare(in, out, max);
        }
    }
}

void TlsConnection::sync(fd_set &in, fd_set &out)
{
    if (m_state->status() == Connecting)
        Connection::sync(in, out);
    else if (m_handshake != HandshakeReady)
        handshake();
    else
        Connection::sync(in, out);
}

} // !irccd
