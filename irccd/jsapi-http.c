/*
 * jsapi-http.c -- Irccd.Http API
 *
 * Copyright (c) 2013-2024 David Demelier <markand@malikania.fr>
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

#include <sys/select.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include <curl/curl.h>

#include <utlist.h>

#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/util.h>

#include "jsapi-http.h"
#include "jsapi-plugin.h"
#include "jsapi-system.h"

#define SIGNATURE       DUK_HIDDEN_SYMBOL("Irccd.Http")
#define CALLBACK        DUK_HIDDEN_SYMBOL("Irccd.Http.callback")
#define TABLE           DUK_HIDDEN_SYMBOL("Irccd.Http.table")

struct op {
	CURL *curl;
	CURLM *multi;
	struct irc_pollable pollable;
	int errn;
	long code;
	char *out;
	size_t outsz;
	FILE *fp;
	void *ptr;
	duk_context *ctx;
};

static int pollable_fd(void *);
static void pollable_want(int *, int *, void *);
static int pollable_sync(int, int, void *);

static void
op_free(struct op *op)
{
	if (op->curl) {
		if (op->multi)
			curl_multi_remove_handle(op->multi, op->curl);

		curl_easy_cleanup(op->curl);
	}
	if (op->multi)
		curl_multi_cleanup(op->multi);
	if (op->fp)
		fclose(op->fp);

	free(op->out);
	free(op);
}

static size_t
op_write(char *ptr, size_t w, size_t n, void *data)
{
	struct op *op = data;

	if (fwrite(ptr, w, n, op->fp) != n) {
		op->errn = errno;
		return 0;
	}

	return n;
}

static struct op *
op_new(duk_context *ctx)
{
	struct op *op;

	op = irc_util_calloc(1, sizeof (*op));
	op->ctx = ctx;

	if (!(op->curl = curl_easy_init()))
		goto enomem;
	if (!(op->multi = curl_multi_init()))
		goto enomem;
	if (!(op->fp = open_memstream(&op->out, &op->outsz)))
		goto enomem;

	curl_easy_setopt(op->curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(op->curl, CURLOPT_WRITEFUNCTION, op_write);
	curl_easy_setopt(op->curl, CURLOPT_WRITEDATA, op);
	curl_easy_setopt(op->curl, CURLOPT_FOLLOWLOCATION, 1L);

	/* CURLOPT_PROTOCOLS is deprecated since 7.85.0. */
#if LIBCURL_VERSION_MAJOR >= 8 || (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 85)
	curl_easy_setopt(op->curl, CURLOPT_PROTOCOLS_STR, "http,https");
#else
	curl_easy_setopt(op->curl, CURLOPT_PROTOCOLS, (long)CURLPROTO_HTTP | CURLPROTO_HTTPS);
#endif

	return op;

enomem:
	op_free(op);
	errno = ENOMEM;

	return NULL;
}

static inline void
op_start(struct op *op)
{
	int pending;

	curl_multi_add_handle(op->multi, op->curl);
	curl_multi_perform(op->multi, &pending);
}

static void
op_complete(struct op *op)
{
	struct irc_plugin *plg = jsapi_plugin_self(op->ctx);

	/* Close file pointer to retrieve data. */
	fclose(op->fp);
	op->fp = NULL;

	/*
	 * Create a result object with the following properties:
	 *
	 * {
	 *   status: 0 on success or errno value on error
	 *   code: HTTP result code
	 *   body: body content if any
	 * }
	 */
	duk_push_heapptr(op->ctx, op->ptr);
	duk_push_object(op->ctx);
	duk_push_int(op->ctx, op->errn);
	duk_put_prop_string(op->ctx, -2, "status");
	duk_push_int(op->ctx, op->code);
	duk_put_prop_string(op->ctx, -2, "code");
	duk_push_string(op->ctx, op->out);
	duk_put_prop_string(op->ctx, -2, "body");

	/*
	 * Remove already the data to avoid leaking too much memory until the
	 * function callback is finalized.
	 */
	free(op->out);
	op->out = NULL;
	op->outsz = 0;

	if (duk_pcall(op->ctx, 1) != 0)
		irc_log_warn("plugin %s: %s", plg->name, duk_to_string(op->ctx, -1));

	duk_pop(op->ctx);

	/*
	 * Now remove the function from the table so it gets deleted at some
	 * point.
	 */
	duk_push_global_stash(op->ctx);
	duk_get_prop_string(op->ctx, -1, TABLE);
	duk_del_prop_heapptr(op->ctx, -1, op->ptr);
	duk_pop_n(op->ctx, 2);
}

static inline void
parsestr(struct op *op, const char *key, int option)
{
	duk_get_prop_string(op->ctx, 0, key);

	if (duk_is_string(op->ctx, -1))
		curl_easy_setopt(op->curl, option, duk_get_string(op->ctx, -1));

	duk_pop(op->ctx);
}

static inline void
parselong(struct op *op, const char *key, int option)
{
	duk_get_prop_string(op->ctx, 0, key);

	if (duk_is_number(op->ctx, -1))
		curl_easy_setopt(op->curl, option, (long)duk_get_number(op->ctx, -1));

	duk_pop(op->ctx);
}

static inline void
parsemethod(struct op *op)
{
	duk_get_prop_string(op->ctx, 0, "method");

	if (duk_is_string(op->ctx, -1))
		curl_easy_setopt(op->curl, CURLOPT_CUSTOMREQUEST, duk_get_string(op->ctx, -1));

	duk_pop(op->ctx);
}

static inline void
parsebody(struct op *op)
{
	duk_get_prop_string(op->ctx, 0, "body");

	if (!duk_is_undefined(op->ctx, -1))
		curl_easy_setopt(op->curl, CURLOPT_POSTFIELDS, duk_to_string(op->ctx, -1));

	duk_pop(op->ctx);
}

static void
op_parse(struct op *op)
{
	parsestr(op, "url", CURLOPT_URL);
	parselong(op, "timeout", CURLOPT_TIMEOUT);
	parsemethod(op);
	parsebody(op);
}

static inline void
op_insert(struct op *op)
{
	op->pollable.data = op;
	op->pollable.fd = pollable_fd;
	op->pollable.want = pollable_want;
	op->pollable.sync = pollable_sync;

	irc_bot_pollable_add(&op->pollable);
}

static int
pollable_fd(void *data)
{
	struct op *op = data;
	fd_set in, out, exc;
	int fd = 0;

	FD_ZERO(&in);
	FD_ZERO(&out);
	FD_ZERO(&exc);

	curl_multi_fdset(op->multi, &in, &out, &exc, &fd);

	return fd;
}

static void
pollable_want(int *frecv, int *fsend, void *data)
{
	struct op *op = data;
	fd_set in, out, exc;
	int maxfd = 0;

	FD_ZERO(&in);
	FD_ZERO(&out);
	FD_ZERO(&exc);

	curl_multi_fdset(op->multi, &in, &out, &exc, &maxfd);

	if (FD_ISSET(maxfd, &in))
		*frecv = 1;
	if (FD_ISSET(maxfd, &out))
		*fsend = 1;
}

static int
pollable_sync(int frecv, int fsend, void *data)
{
	(void)frecv;
	(void)fsend;

	CURLMsg *msg;
	struct op *op = data;
	int pending, msgsz;

	/*
	 * CURL does its own job reading/sending without taking action on what
	 * have been found.
	 */
	if (curl_multi_perform(op->multi, &pending) < 0) {
		op->errn = EINVAL;
		op_complete(op);
		return -1;
	}

	/* We only have one handle so we can just assume 0 means complete. */
	if (pending)
		return 0;

	while ((msg = curl_multi_info_read(op->multi, &msgsz))) {
		if (msg->msg != CURLMSG_DONE)
			continue;

		switch (msg->data.result) {
		case CURLE_OPERATION_TIMEDOUT:
			op->errn = ETIMEDOUT;
			break;
		case CURLE_OK:
			op->errn = 0;
			curl_easy_getinfo(op->curl, CURLINFO_RESPONSE_CODE,
			    &op->code);
			break;
		default:
			break;
		}

		op_complete(op);
		return -1;
	}

	op->errn = EINVAL;
	op_complete(op);

	return 0;
}

static int
Http_destructor(duk_context *ctx)
{
	struct op *op;

	duk_get_prop_string(ctx, 0, SIGNATURE);

	if ((op = duk_to_pointer(ctx, -1))) {
		duk_push_global_stash(ctx);
		duk_get_prop_string(ctx, -1, TABLE);
		duk_del_prop_heapptr(ctx, -1, op->ptr);
		duk_pop_n(ctx, 2);
		op_free(duk_to_pointer(ctx, -1));
	}

	duk_del_prop_string(ctx, 0, SIGNATURE);
	duk_pop(ctx);

	return 0;
}

static int
Http_request(duk_context *ctx)
{
	struct op *op;

	duk_require_object(ctx, 0);
	duk_require_callable(ctx, 1);

	if (!(op = op_new(ctx)))
		jsapi_system_raise(ctx);

	op_parse(op);
	op_start(op);
	op_insert(op);

	/*
	 * We attach the HTTP operation to the function callback argument with
	 * a specific finalizer.
	 */
	duk_dup(ctx, 1);
	duk_push_pointer(ctx, op);
	duk_put_prop_string(ctx, -2, SIGNATURE);
	duk_push_c_function(ctx, Http_destructor, 1);
	duk_set_finalizer(ctx, -2);
	duk_pop(ctx);

	/* Store the callback in a global table. */
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, TABLE);
	duk_dup(ctx, 1);
	duk_put_prop_heapptr(ctx, -2, (op->ptr = duk_get_heapptr(ctx, 1)));
	duk_pop_n(ctx, 2);

	return 0;
}

static const duk_function_list_entry functions[] = {
	 { "request",   Http_request,   2 },
	 { NULL,        NULL,           0 },
};

void
jsapi_http_load(duk_context *ctx)
{
	assert(ctx);

	duk_get_global_string(ctx, "Irccd");
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, functions);
	duk_put_prop_string(ctx, -2, "Http");
	duk_pop(ctx);

	duk_push_global_stash(ctx);
	duk_push_object(ctx);
	duk_put_prop_string(ctx, -2, TABLE);
	duk_pop(ctx);
}
