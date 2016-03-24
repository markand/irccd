/*
 * js.cpp -- JavaScript C++14 wrapper for Duktape
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

#include "js.h"

using namespace std::string_literals;

namespace irccd {

namespace js {

ErrorInfo Context::error(int index)
{
	ErrorInfo error;

	index = duk_normalize_index(m_handle.get(), index);

	error.name = optionalProperty<std::string>(index, "name", "");
	error.message = optionalProperty<std::string>(index, "message", "");
	error.fileName = optionalProperty<std::string>(index, "fileName", "");
	error.lineNumber = optionalProperty<int>(index, "lineNumber", 0);
	error.stack = optionalProperty<std::string>(index, "stack", "");

	return error;
}

void Context::pcall(unsigned nargs)
{
	if (duk_pcall(m_handle.get(), nargs) != 0) {
		ErrorInfo info = error(-1);
		duk_pop(m_handle.get());

		throw info;
	}
}

void Context::peval()
{
	if (duk_peval(m_handle.get()) != 0) {
		ErrorInfo info = error(-1);
		duk_pop(m_handle.get());

		throw info;
	}
}

void TypeInfo<Function>::push(Context &ctx, Function fn)
{
	/* 1. Push function wrapper */
	duk_push_c_function(ctx, [] (duk_context *ctx) -> duk_ret_t {
		Context context(ctx);

		duk_push_current_function(ctx);
		duk_get_prop_string(ctx, -1, "\xff""\xff""js-func");
		Function *f = static_cast<Function *>(duk_to_pointer(ctx, -1));
		duk_pop_2(ctx);

		return static_cast<duk_ret_t>(f->function(context));
	}, fn.nargs);

	/* 2. Store the moved function */
	duk_push_pointer(ctx, new Function(std::move(fn)));
	duk_put_prop_string(ctx, -2, "\xff""\xff""js-func");

	/* 3. Store deletion flags */
	duk_push_boolean(ctx, false);
	duk_put_prop_string(ctx, -2, "\xff""\xff""js-deleted");

	/* 4. Push and set a finalizer */
	duk_push_c_function(ctx, [] (duk_context *ctx) {
		duk_get_prop_string(ctx, 0, "\xff""\xff""js-deleted");

		if (!duk_to_boolean(ctx, -1)) {
			duk_push_boolean(ctx, true);
			duk_put_prop_string(ctx, 0, "\xff""\xff""js-deleted");
			duk_get_prop_string(ctx, 0, "\xff""\xff""js-func");
			delete static_cast<Function *>(duk_to_pointer(ctx, -1));
			duk_pop(ctx);
		}

		duk_pop(ctx);

		return 0;
	}, 1);
	duk_set_finalizer(ctx, -2);
}

} // !js

} // !irccd
