/*
 * transport-command.h -- command to execute server side
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

#ifndef _IRCCD_TRANSPORT_COMMAND_H_
#define _IRCCD_TRANSPORT_COMMAND_H_

#include <json.h>

#include "irccd.h"
#include "transport-client.h"

namespace irccd {

class Irccd;
class TransportClient;

namespace json {

class Value;

} // !json

/**
 * @class TransportCommand
 * @brief Command to execute server side.
 */
class TransportCommand {
public:
	/**
	 * Default constructor.
	 */
	TransportCommand() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~TransportCommand() = default;

	/**
	 * Execute the client request, the implementation can throw anything.
	 *
	 * @param irccd the irccd instance
	 * @param tc the client
	 * @param object the request
	 */
	virtual void exec(Irccd &irccd, TransportClient &tc, const json::Value &object) const = 0;
};

} // !irccd

#endif // _IRCCD_TRANSPORT_COMMAND_H_
