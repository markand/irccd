/*
 * transport_client.hpp -- server side transport clients
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_TRANSPORT_CLIENT_HPP
#define IRCCD_TRANSPORT_CLIENT_HPP

#include <deque>
#include <memory>
#include <functional>
#include <string>
#include <utility>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "json.hpp"

#include "errors.hpp"

namespace irccd {

class transport_server;

/**
 * \brief Error for transports.
 */
enum class transport_error : int {
    invalid_auth = 1,       //! invalid authentication
    invalid_message,        //! client has sent an invalid message
    incomplete_message      //!< message requires more parameter
};

/**
 * \brief Abstract transport client class.
 *
 * This class is responsible of receiving/sending data.
 */
class transport_client : public std::enable_shared_from_this<transport_client> {
public:
    /**
     * Callback on receive operation.
     */
    using recv_t = std::function<void (const nlohmann::json&, const boost::system::error_code&)>;

    /**
     * Callback on send operation.
     */
    using send_t = std::function<void (const boost::system::error_code&)>;

protected:
    /**
     * Handler for do_recv.
     *
     * The implementation should read until \r\n\r\n is found.
     */
    using do_recv_handler_t = std::function<void (const boost::system::error_code&, std::size_t)>;

    /**
     * Handler for do_send.
     *
     * The implementation must send the whole message.
     */
    using do_send_handler_t = std::function<void (const boost::system::error_code&, std::size_t)>;

    /**
     * Input buffer.
     */
    using input_t = boost::asio::streambuf;

private:
    using output_t = std::deque<std::pair<std::string, send_t>>;

    enum class state_t {
        authenticating,
        ready,
        closing
    } state_{state_t::authenticating};

    input_t input_;
    output_t output_;
    transport_server& parent_;

    void close();
    void flush();

protected:
    /**
     * Start a read operation, the implementation must start reading without
     * any checks.
     *
     * \pre handler is not null
     * \param input the input buffer
     * \param handler the completion handler
     */
    virtual void do_recv(input_t& input, do_recv_handler_t handler) = 0;

    /**
     * Start a send operation, the implementation has no checks to perform
     * because it is already done in transport_client functions.
     *
     * The message buffer remains valid until completion is complete.
     *
     * \pre message is not empty
     * \pre handler is not null
     * \param message the data to send
     * \param handler the completion handler
     */
    virtual void do_send(const std::string& message, do_send_handler_t handler) = 0;

public:
    /**
     * Default constructor.
     */
    inline transport_client(transport_server& server) noexcept
        : parent_(server)
    {
    }

    /**
     * Virtual destructor defaulted.
     */
    virtual ~transport_client() noexcept = default;

    /**
     * Get the transport server parent.
     *
     * \return the parent
     */
    inline const transport_server& parent() const noexcept
    {
        return parent_;
    }

    /**
     * Overloaded function.
     *
     * \return the parent
     */
    inline transport_server& parent() noexcept
    {
        return parent_;
    }

    /**
     * Get the current client state.
     *
     * \return the state
     */
    inline state_t state() const noexcept
    {
        return state_;
    }

    /**
     * Set the client state.
     *
     * \param state the new state
     */
    inline void set_state(state_t state) noexcept
    {
        state_ = state;
    }

    /**
     * Start a receive operation.
     *
     * \param handler the handler
     */
    void recv(recv_t handler);

    /**
     * Send or postpone some data to the client.
     *
     * If there are pending data, the operation will be ran once all other
     * messages has been sent.
     *
     * \note if state is closing, no data is sent
     * \pre data.is_object()
     * \param data the message to send
     * \param handler the optional completion handler
     */
    void send(const nlohmann::json& data, send_t handler = nullptr);

    /**
     * Convenient success message.
     *
     * \param cname the command name
     * \param handler the optional handler
     */
    inline void success(const std::string& cname, send_t handler = nullptr)
    {
        assert(!cname.empty());

        send({{ "command", cname }}, std::move(handler));
    }

    /**
     * Send a error message, the state is set to closing.
     *
     * The invocation is similar to:
     *
     * ````cpp
     * set_state(state_t::closing);
     * send(message, handler);
     * ````
     *
     * \pre message is not null
     * \pre data.is_object()
     * \param message the error message
     * \param handler the handler
     */
    void error(const nlohmann::json& data, send_t handler = nullptr);

    /**
     * Convenient error overload.
     *
     * \param cname the command name
     * \param reason the reason string
     * \param handler the optional handler
     */
    inline void error(const std::string& cname, const std::string& reason, send_t handler = nullptr)
    {
        assert(!cname.empty());

        error({
            { "command",    cname   },
            { "error",      reason  }
        }, std::move(handler));
    }

    /**
     * Convenient error overload.
     *
     * \param cname the command name
     * \param reason the error code
     * \param handler the optional handler
     */
    inline void error(const std::string& cname, network_error reason, send_t handler)
    {
        assert(!cname.empty());

        error({
            { "command",    cname                       },
            { "error",      static_cast<int>(reason)    }
        }, std::move(handler));
    }
};

/**
 * \brief Basic implementation for IP/Tcp and local sockets
 *
 * This class implements an recv/send function for:
 *
 *   - boost::asio::ip::tcp
 *   - boost::asio::local::stream_protocol
 *   - boost::asio::ssl::stream
 */
template <typename Socket>
class basic_transport_client : public transport_client {
protected:
    Socket socket_;

public:
    /**
     * Constructor.
     *
     * \param sock the socket
     */
    template <typename... Args>
    inline basic_transport_client(transport_server& parent, Args&&... args) noexcept
        : transport_client(parent)
        , socket_(std::forward<Args>(args)...)
    {
    }

    /**
     * Get the underlying socket.
     *
     * \return the socket
     */
    inline const Socket& socket() const noexcept
    {
        return socket_;
    }

    /**
     * Overloaded function.
     *
     * \return the socket
     */
    inline Socket& socket() noexcept
    {
        return socket_;
    }

    /**
     * \copydoc transport_client::do_recv
     */
    void do_recv(input_t& input, do_recv_handler_t handler) override;

    /**
     * \copydoc transport_client::do_send
     */
    void do_send(const std::string& data, do_send_handler_t handler) override;
};

template <typename Socket>
void basic_transport_client<Socket>::do_recv(input_t& input, do_recv_handler_t handler)
{
    auto self = shared_from_this();

    boost::asio::async_read_until(socket_, input, "\r\n\r\n", [this, self, handler] (auto code, auto xfer) {
        handler(code, xfer);
    });
}

template <typename Socket>
void basic_transport_client<Socket>::do_send(const std::string& data, do_send_handler_t handler)
{
    auto self = shared_from_this();

    boost::asio::async_write(socket_, boost::asio::buffer(data), [this, self, handler] (auto code, auto xfer) {
        handler(code, xfer);
    });
}

/**
 * \brief Secure layer client.
 */
class tls_transport_client : public basic_transport_client<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> {
public:
    /**
     * Construct a secure layer client.
     *
     * \param parent the parent transport
     * \param service the service to use
     * \param context the context to reuse
     */
    tls_transport_client(transport_server& parent,
                         boost::asio::io_service& service,
                         boost::asio::ssl::context& context)
        : basic_transport_client(parent, service, context)
    {
    }
};

/**
 * Get the transport category.
 *
 * \return the singleton category
 */
const boost::system::error_category& transport_category() noexcept;

/**
 * Wrap the creation of an error_code based on transport_server::error.
 *
 * \param e the transport_server error code
 * \return a boost::system::error_code with transport_category
 */
boost::system::error_code make_error_code(transport_error e) noexcept;

} // !irccd

namespace boost {

namespace system {

template <>
struct is_error_code_enum<irccd::transport_error> : public std::true_type {
};

} // !system

} // !boost

#endif // !IRCCD_TRANSPORT_CLIENT_HPP
