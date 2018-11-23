/*
 * modules.cpp -- doxygen modules page
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

/**
 * \defgroup daemon irccd::daemon
 * \brief Module libirccd-daemon
 */

/**
 * \defgroup daemon-transports transports
 * \ingroup daemon
 * \brief Servers, clients and commands.
 */

/**
 * \defgroup daemon-servers servers
 * \ingroup daemon
 * \brief IRC server, events and functions.
 */

/**
 * \defgroup daemon-plugins plugins
 * \ingroup daemon
 * \brief Plugin objects and functions.
 */

/**
 * \defgroup daemon-loggers loggers
 * \ingroup daemon
 * \brief Log mechanism.
 */

/**
 * \defgroup daemon-loggers-sinks sinks
 * \ingroup daemon-loggers
 * \brief Predefined logger sinks.
 */

/**
 * \brief Specialized loggable traits.
 * \defgroup daemon-loggers-traits traits
 * \ingroup daemon-loggers
 */


/**
 * \defgroup daemon-rules rules
 * \ingroup daemon
 * \brief Rule objects.
 */

/**
 * \defgroup daemon-utilites utilities
 * \ingroup daemon
 * \brief Utilities.
 */

/**
 * \defgroup daemon-services services
 * \ingroup daemon
 * \brief Irccd services.
 */

/**
 * \defgroup ctl irccd::ctl
 * \brief Module libirccd-ctl
 */

/**
 * \defgroup core irccd
 * \brief Module libirccd
 */

/**
 * \defgroup core-networking networking
 * \ingroup core
 * \brief Networking support.
 *
 * Each irccd instance is controllable via sockets using JSON messages.
 *
 * This mechanism is offered via the triplet stream/acceptor/connector. Irccd
 * uses different acceptors to wait for clients to connect and then construct
 * a stream of it. Once ready, streams are ready to receive and send messages.
 *
 * On the client side (e.g. irccdctl), a generic connector is created to connect
 * to the irccd instance. Once ready, a stream is also created and ready to
 * perform the same receive and send messages.
 *
 * By default, irccd provides predefined implementations for TCP/IP, local unix
 * sockets and optionally TLS over those.
 */

/**
 * \defgroup core-streams streams
 * \ingroup core-networking
 * \brief Generic I/O streams.
 */

/**
 * \defgroup core-acceptors acceptors
 * \ingroup core-networking
 * \brief Generic I/O acceptors.
 */

/**
 * \defgroup core-connectors connectors
 * \ingroup core-networking
 * \brief Generic I/O connectors.
 */

/**
 * \defgroup js irccd::js
 * \brief Javascript support.
 */

/**
 * \defgroup js-api api
 * \ingroup js
 * \brief Javascript APIs.
 */