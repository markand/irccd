/*
 * js-server.h -- Irccd.Server API
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

#ifndef IRCCD_JS_SERVER_H
#define IRCCD_JS_SERVER_H

#include <irccd/server.h>

#include "js.h"

namespace irccd {

void loadJsServer(duk::ContextPtr ctx);

namespace duk {

template <>
class TypeTraits<irccd::Server> {
public:
	static inline void prototype(ContextPtr ctx)
	{
		getGlobal<void>(ctx, "Irccd");
		getGlobal<void>(ctx, "Server");
		getProperty<void>(ctx, -1, "prototype");
		remove(ctx, -2);
		remove(ctx, -2);
	}

	static inline std::string name()
	{
		return "\xff""\xff""Server";
	}

	static inline std::vector<std::string> inherits()
	{
		return {};
	}
};

} // !duk

} // !irccd

#endif // !IRCCD_JS_SERVER_H
