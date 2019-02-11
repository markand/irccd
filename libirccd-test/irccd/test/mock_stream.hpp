/*
 * mock_stream.hpp -- mock stream
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_TEST_MOCK_STREAM_HPP
#define IRCCD_TEST_MOCK_STREAM_HPP

/**
 * \file mock_stream.hpp
 * \brief Mock stream.
 */

#include <irccd/stream.hpp>

#include "mock.hpp"

namespace irccd::test {

/**
 * \brief Mock stream.
 */
class mock_stream : public stream, public mock {
public:
	/**
	 * \copydoc stream::recv
	 */
	void recv(recv_handler handler) override;

	/**
	 * \copydoc stream::send
	 */
	void send(const nlohmann::json& json, send_handler handler) override;
};

} // !irccd::test

#endif // !IRCCD_TEST_MOCK_STREAM_HPP
