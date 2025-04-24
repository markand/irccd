/*
 * jsapi-http.c -- Irccd.Http API
 *
 * Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

#include <ev.h>

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

struct request {
	/* CURL types. */
	CURL *curl;
	CURLM *multi;
	int error;
	long code;

	/* CURL watcher coupled to libev. */
	struct ev_io io;

	/* Output buffer. */
	char *out;
	size_t outsz;
	FILE *fp;

	/* Duktape context and callback function. */
	duk_context *ctx;
	void *addr;
};

static int Http_destructor(duk_context *);

static void
request_free(struct request *req)
{
	if (!req)
		return;

	ev_io_stop(irc_bot_loop(), &req->io);

	if (req->curl) {
		if (req->multi)
			curl_multi_remove_handle(req->multi, req->curl);

		curl_easy_cleanup(req->curl);
	}
	if (req->multi)
		curl_multi_cleanup(req->multi);
	if (req->fp)
		fclose(req->fp);

	free(req->out);
	free(req);
}

static void
request_attach(struct request *req, duk_context *ctx, duk_idx_t index)
{
	req->ctx = ctx;

	/*
	 * The function argument provided as index is the Javascript callback
	 * that we will invoke when the HTTP request terminates.
	 *
	 * Because the user pass a function that is not tighted to a value it
	 * may be collected early. As such, we "copy" by adding a reference into
	 * the stash and retrieve it later on.
	 *
	 * We must also set a finalizer in case the HTTP request is pending but
	 * the Javascript context is being closed (e.g. when stopping irccd).
	 */
	req->addr = duk_get_heapptr(ctx, index);
	duk_push_pointer(ctx, req);
	duk_put_prop_string(ctx, index, SIGNATURE);
	duk_push_c_function(ctx, Http_destructor, 1);
	duk_set_finalizer(ctx, index);

	/* Now link this callback into the heap stash using its "pointer". */
	duk_push_heap_stash(ctx);
	duk_push_pointer(ctx, req->addr);
	duk_dup(ctx, index);
	duk_put_prop(ctx, -3);
	duk_pop(ctx);
}

static inline void
request_detach(struct request *req)
{
	duk_push_heap_stash(req->ctx);
	duk_del_prop_heapptr(req->ctx, -1, req->addr);
	duk_pop(req->ctx);
}

static size_t
request_write(char *ptr, size_t w, size_t n, void *data)
{
	struct request *req = data;

	if (fwrite(ptr, w, n, req->fp) != n) {
		req->error = errno;
		return 0;
	}

	return n;
}

static void
request_set_events(struct request *req)
{
	fd_set in, out, except;
	int flags = 0, fd = 0;

	FD_ZERO(&in);
	FD_ZERO(&out);
	FD_ZERO(&except);

	curl_multi_fdset(req->multi, &in, &out, &except, &fd);

	if (FD_ISSET(fd, &in))
		flags |= EV_READ;
	if (FD_ISSET(fd, &out))
		flags |= EV_WRITE;

	if (req->io.events != flags) {
		ev_io_stop(irc_bot_loop(), &req->io);
		ev_io_set(&req->io, fd, flags);
		ev_io_start(irc_bot_loop(), &req->io);
	}
}

static void
request_complete(struct request *req)
{
	struct irc_plugin *plg;

	/* Close file pointer to retrieve data. */
	fflush(req->fp);
	fclose(req->fp);
	req->fp = NULL;

	/* Get reference to the plugin. */
	plg = jsapi_plugin_self(req->ctx);

	/*
	 * Create a result object with the following properties:
	 *
	 * {
	 *   status: 0 on success or errno value on error
	 *   code: HTTP result code
	 *   body: body content if any
	 * }
	 */
	duk_push_heapptr(req->ctx, req->addr);
	duk_push_object(req->ctx);
	duk_push_int(req->ctx, req->error);
	duk_put_prop_string(req->ctx, -2, "status");
	duk_push_int(req->ctx, req->code);
	duk_put_prop_string(req->ctx, -2, "code");
	duk_push_string(req->ctx, req->out);
	duk_put_prop_string(req->ctx, -2, "body");

	/*
	 * Remove already the data to avoid leaking too much memory until the
	 * function callback is finalized.
	 */
	free(req->out);
	req->out = NULL;
	req->outsz = 0;

	if (duk_pcall(req->ctx, 1) != 0)
		irc_log_warn("plugin %s: %s", plg->name, duk_to_string(req->ctx, -1));

	duk_pop(req->ctx);

	/* Unlink the object from the stash. */
	request_detach(req);
}

static void
request_io_cb(struct ev_loop *loop, struct ev_io *self, int revents)
{
	(void)loop;
	(void)revents;

	struct request *req;
	CURLMsg *msg;
	int msgsz, pending = 0;

	req = IRC_UTIL_CONTAINER_OF(self, struct request, io);

	/*
	 * CURL does its own job reading/sending without taking action on socket
	 * events.
	 */
	if (curl_multi_perform(req->multi, &pending) < 0) {
		req->error = EINVAL;
		goto end;
	}

	/* We only have one handle so we can just assume 0 means complete. */
	if (pending)
		goto end;

	while ((msg = curl_multi_info_read(req->multi, &msgsz)) && msg->msg != CURLMSG_DONE)
		continue;

	/* No CURL response it seems. */
	if (!msg) {
		req->error = EINVAL;
		goto end;
	}

	switch (msg->data.result) {
	case CURLE_OPERATION_TIMEDOUT:
		req->error = ETIMEDOUT;
		break;
	case CURLE_OK:
		req->error = 0;
		curl_easy_getinfo(req->curl, CURLINFO_RESPONSE_CODE, &req->code);
		break;
	default:
		break;
	}

end:
	if (pending)
		request_set_events(req);
	else
		request_complete(req);
}

static struct request *
request_new(void)
{
	struct request *req;

	req = irc_util_calloc(1, sizeof (*req));

	if (!(req->curl = curl_easy_init()))
		goto enomem;
	if (!(req->multi = curl_multi_init()))
		goto enomem;
	if (!(req->fp = open_memstream(&req->out, &req->outsz)))
		goto enomem;

	curl_easy_setopt(req->curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(req->curl, CURLOPT_WRITEFUNCTION, request_write);
	curl_easy_setopt(req->curl, CURLOPT_WRITEDATA, req);
	curl_easy_setopt(req->curl, CURLOPT_FOLLOWLOCATION, 1L);

	/* CURLOPT_PROTOCOLS is deprecated since 7.85.0. */
#if LIBCURL_VERSION_MAJOR >= 8 || (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 85)
	curl_easy_setopt(req->curl, CURLOPT_PROTOCOLS_STR, "http,https");
#else
	curl_easy_setopt(req->curl, CURLOPT_PROTOCOLS, (long)CURLPROTO_HTTP | CURLPROTO_HTTPS);
#endif

	return req;

enomem:
	request_free(req);
	errno = ENOMEM;

	return NULL;
}

static inline void
request_start(struct request *req)
{
	int pending;

	curl_multi_add_handle(req->multi, req->curl);
	curl_multi_perform(req->multi, &pending);

	ev_init(&req->io, request_io_cb);

	request_set_events(req);
}

static inline void
request_parsestr(struct request *req, const char *key, int option)
{
	duk_get_prop_string(req->ctx, 0, key);

	if (duk_is_string(req->ctx, -1))
		curl_easy_setopt(req->curl, option, duk_get_string(req->ctx, -1));

	duk_pop(req->ctx);
}

static inline void
request_parselong(struct request *req, const char *key, int option)
{
	duk_get_prop_string(req->ctx, 0, key);

	if (duk_is_number(req->ctx, -1))
		curl_easy_setopt(req->curl, option, (long)duk_get_number(req->ctx, -1));

	duk_pop(req->ctx);
}

static inline void
request_parsemethod(struct request *req)
{
	duk_get_prop_string(req->ctx, 0, "method");

	if (duk_is_string(req->ctx, -1))
		curl_easy_setopt(req->curl, CURLOPT_CUSTOMREQUEST, duk_get_string(req->ctx, -1));

	duk_pop(req->ctx);
}

static inline void
request_parsebody(struct request *req)
{
	duk_get_prop_string(req->ctx, 0, "body");

	if (!duk_is_undefined(req->ctx, -1))
		curl_easy_setopt(req->curl, CURLOPT_POSTFIELDS, duk_to_string(req->ctx, -1));

	duk_pop(req->ctx);
}

static inline void
request_parse(struct request *req)
{
	request_parsestr(req, "url", CURLOPT_URL);
	request_parselong(req, "timeout", CURLOPT_TIMEOUT);
	request_parsemethod(req);
	request_parsebody(req);
}

static int
Http_destructor(duk_context *ctx)
{
	duk_get_prop_string(ctx, 0, SIGNATURE);
	request_free(duk_to_pointer(ctx, -1));
	duk_del_prop_string(ctx, 0, SIGNATURE);
	duk_pop(ctx);

	return 0;
}

static int
Http_request(duk_context *ctx)
{
	struct request *req;

	duk_require_object(ctx, 0);
	duk_require_callable(ctx, 1);

	if (!(req = request_new()))
		jsapi_system_raise(ctx);

	request_attach(req, ctx, 1);
	request_parse(req);
	request_start(req);

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
