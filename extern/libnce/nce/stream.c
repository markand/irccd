/*
 * stream.c -- file descriptor support
 *
 * Copyright (c) 2025-2026 David Demelier <markand@malikania.fr>
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

#define _GNU_SOURCE
#include <sys/socket.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stream.h"

#define NCE_STREAM_ROOM_IN(Ch) \
        ((Ch)->in == NULL ? 0 : ((Ch)->in_cap - (Ch)->in_len))
#define NCE_STREAM_ROOM_OUT(Ch) \
        ((Ch)->out == NULL ? 0 : ((Ch)->out_cap  - (Ch)->out_len))

/*
 * Remove data from the specified buffer by moving data after what has been
 * consumed.
 */
static inline void
nce_stream_drain_buf(unsigned char *buf, size_t *buf_len, size_t buf_cap, size_t amount)
{
	memmove(buf, &buf[amount], buf_cap - amount);
	*buf_len -= amount;
}

/*
 * Update stream watcher flags depending on the incoming/outgoing buffers.
 *
 * If incoming is full, deactivate readable
 * If outgoing is non-empty, activate writable
 */
static void
nce_stream_io_set(EV_P_ struct nce_stream *stream, int enable)
{
	int events = 0;

	if (!enable && !nce_io_active(&stream->io_fd))
		return;

	if (stream->in && stream->in_len < stream->in_cap)
		events |= EV_READ;
	if (stream->out && stream->out_len)
		events |= EV_WRITE;

	nce_io_stop(EV_A_ &stream->io_fd);

	if (events) {
		nce_io_set(&stream->io_fd, stream->fd, events);
		nce_io_start(EV_A_ &stream->io_fd);
	}
}

/*
 * Default stream::receive function for non-sockets types.
 */
static ssize_t
nce_stream_regular_read(EV_P_ struct nce_stream *stream, void *data, size_t len)
{
	ssize_t rc = 0;

	switch ((rc = read(stream->fd, data, len))) {
	case 0:
		rc = -EPIPE;
		break;
	case -1:
		if (errno != EAGAIN)
			rc = -errno;
		else
			rc = 0;
		break;
	default:
		break;
	}

	return rc;
}

/*
 * Default stream::send function for non-sockets types.
 */
static ssize_t
nce_stream_regular_write(EV_P_ struct nce_stream *stream, const void *data, size_t len)
{
	ssize_t rc = 0;

	switch ((rc = write(stream->fd, data, len))) {
	case -1:
		if (errno != EAGAIN)
			rc = -errno;
		break;
	default:
		break;
	}

	return rc;
}

static void
nce_stream_regular_close(EV_P_ struct nce_stream *stream)
{
	close(stream->fd);
}

/*
 * Default stream::receive function for sockets types.
 */
static ssize_t
nce_stream_socket_read(EV_P_ struct nce_stream *stream, void *data, size_t len)
{
	ssize_t rc = 0;

	switch ((rc = recv(stream->fd, data, len, MSG_NOSIGNAL))) {
	case 0:
		rc = -EPIPE;
		break;
	case -1:
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			rc = -errno;
		else
			rc = 0;
		break;
	default:
		break;
	}

	return rc;
}

/*
 * Default stream::send function for non-sockets types.
 */
static ssize_t
nce_stream_socket_write(EV_P_ struct nce_stream *stream, const void *data, size_t len)
{
	ssize_t rc = 0;

	switch ((rc = send(stream->fd, data, len, MSG_NOSIGNAL))) {
	case -1:
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			rc = -errno;
		break;
	default:
		break;
	}

	return rc;
}

static void
nce_stream_socket_close(EV_P_ struct nce_stream *stream)
{
	close(stream->fd);
}

static int
nce_stream_io_read(EV_P_ struct nce_stream *stream)
{
	size_t room;
	ssize_t rc;

	room = NCE_STREAM_ROOM_IN(stream);

	/*
	 * Watcher flags should be unset already but let's assume we have this
	 * even nevertheless. Just ignore for now.
	 */
	if (room == 0)
		return 0;

	rc = stream->ops->read(EV_A_ stream, &stream->in[stream->in_len], room);

	if (rc > 0) {
		/* Increase current buffer length. */
		stream->in_len += rc;
		assert(stream->in_len <= stream->in_cap);

		/* Update possible new events. */
		nce_stream_io_set(EV_A_ stream, 1);

		/* We return 0 on success. */
		rc = 0;
	}

	return (int)rc;
}

static int
nce_stream_io_write(EV_P_ struct nce_stream *stream)
{
	ssize_t rc;

	/*
	 * Similarly to recv operation, ignore if we have been fired with
	 * nothing to send.
	 */
	if (stream->out_len == 0)
		return 0;

	rc = stream->ops->write(EV_A_ stream, stream->out, stream->out_len);

	if (rc < 0)
		return rc;

	/*
	 * The write function should never return 0 but let's assume user
	 * function can.
	 */
	if (rc > 0) {
		/* First off, drain what we've just sent. */
		assert((size_t)rc <= stream->out_cap);
		nce_stream_drain_buf(stream->out, &stream->out_len, stream->out_cap, rc);

		/* Update possible new events. */
		nce_stream_io_set(EV_A_ stream, 1);

		/* We return 0 on success. */
		rc = 0;
	}

	return (int)rc;
}

static inline ssize_t
nce_stream_extract(EV_P_ struct nce_stream *ch, void *data, size_t length)
{
	if (ch->in == NULL)
		return 0;

	memcpy(data, ch->in, length);
	nce_stream_drain_buf(ch->in, &ch->in_len, ch->in_cap, length);
	nce_stream_io_set(EV_A_ ch, 0);

	return length;
}

static inline void
nce_stream_insert(EV_P_ struct nce_stream *ch, const void *data, size_t length)
{
	assert(ch->out);

	memcpy(&ch->out[ch->out_len], data, length);
	ch->out_len += length;
	nce_stream_io_set(EV_A_ ch, 0);
}

/*
 * Allocate dynamic memory inside the stream input/output buffer if
 * requesed by the user (buffer NULL but capacity > 0).
 *
 * If that fails, we just keep buffers NULL but do not edit the capacity as they
 * are public fields. However, any I/O operation will fail.
 *
 * If user really wants to make sure allocation succeeded, it should do it
 * manually.
 */
static void
nce_stream_allocate(unsigned char **buf, size_t buf_cap, int *buf_dyn)
{
	if (*buf == NULL && buf_cap != 0)
		if ((*buf = malloc(buf_cap)))
			*buf_dyn = 1;
}

/*
 * Deallocate input/output buffer if the stream owns the memory.
 *
 * We keep the capacity to the original value so that a call to ::stream_start
 * will realloc it on the fly.
 */
static inline void
nce_stream_deallocate(unsigned char **buf, size_t *buf_len, int *buf_dyn)
{
	if (*buf_dyn) {
		free(*buf);
		*buf = NULL;
		*buf_dyn = 0;
		*buf_len = 0;
	}
}

const struct nce_stream_ops nce_stream_ops_regular = {
	.read  = nce_stream_regular_read,
	.write = nce_stream_regular_write,
	.close = nce_stream_regular_close
};

const struct nce_stream_ops nce_stream_ops_socket = {
	.read  = nce_stream_socket_read,
	.write = nce_stream_socket_write,
	.close = nce_stream_socket_close
};

void
nce_stream_start(EV_P_ struct nce_stream *ios)
{
	assert(ios);
	assert(ios->ops);

	/*
	 * Reset input/output length. If user didn't zero-initialized the
	 * stream we can't make a check on it.
	 */
	ios->in_len  = 0;
	ios->out_len = 0;

	/* Allocate buffers if required. */
	nce_stream_allocate(&ios->in,  ios->in_cap,  &ios->in_dyn);
	nce_stream_allocate(&ios->out, ios->out_cap, &ios->out_dyn);

	/* Enable I/O events if has already data. */
	nce_stream_io_set(EV_A_ ios, 1);
}

int
nce_stream_active(const struct nce_stream *ios)
{
	assert(ios);

	return nce_io_active(&ios->io_fd);
}

void
nce_stream_stop(EV_P_ struct nce_stream *ios)
{
	assert(ios);

	if (!nce_stream_active(ios))
		return;

	nce_io_stop(EV_A_ &ios->io_fd);

	/* Cleanup buffers if required. */
	nce_stream_deallocate(&ios->in,  &ios->in_len,  &ios->in_dyn);
	nce_stream_deallocate(&ios->out, &ios->out_len, &ios->out_dyn);

	if (ios->close) {
#if !defined(NDEBUG)
		if (!ios->ops->close) {
			fprintf(stderr, "close option but ops->close is NULL\n");
			abort();
		}
#endif
		ios->ops->close(EV_A_ ios);
	}
}

ssize_t
nce_stream_read(EV_P_ struct nce_stream *stream, void *data, size_t length)
{
	assert(stream);
	assert(data);

	if (stream->fd == -1)
		return -EBADF;

	length = length > stream->in_len ? stream->in_len : length;

	return nce_stream_extract(EV_A_ stream, data, length);
}

int
nce_stream_write(EV_P_ struct nce_stream *stream, const void *data, size_t length)
{
	size_t room;

	if (stream->fd == -1)
		return -EBADF;

	room = NCE_STREAM_ROOM_OUT(stream);
	length = length > room ? room : length;

	if (room == 0 || length > room)
		return -ENOBUFS;

	nce_stream_insert(EV_A_ stream, data, length);

	return length;
}

ssize_t
nce_stream_pull(EV_P_ struct nce_stream *stream, void *data, size_t length)
{
	assert(stream);
	assert(data);

	if (stream->fd == -1)
		return -EBADF;
	if (stream->in_len < length)
		return -EAGAIN;

	return nce_stream_extract(EV_A_ stream, data, length);
}

ssize_t
nce_stream_push(EV_P_ struct nce_stream *stream, const void *data, size_t length)
{
	assert(stream);
	assert(data);

	if (stream->fd == -1)
		return -EBADF;

	if (length > NCE_STREAM_ROOM_OUT(stream))
		return -ENOBUFS;

	nce_stream_insert(EV_A_ stream, data, length);

	return length;
}

int
nce_stream_printf(EV_P_ struct nce_stream *stream, const char *fmt, ...)
{
	assert(stream);
	assert(fmt);

	va_list ap;
	int rc;

	va_start(ap, fmt);
	rc = nce_stream_vprintf(EV_A_ stream, fmt, ap);
	va_end(ap);

	return rc;
}

int
nce_stream_vprintf(EV_P_ struct nce_stream *stream, const char *fmt, va_list ap)
{
	assert(stream);
	assert(fmt);

	char *line = NULL;
	ssize_t rc;

	rc = vasprintf(&line, fmt, ap);

	if (rc < 0)
		rc = -ENOMEM;
	else {
		rc = nce_stream_push(EV_A_ stream, line, rc);
		free(line);
	}

	return rc;
}

void
nce_stream_clear(EV_P_ struct nce_stream *stream, enum nce_stream_clear clear)
{
	assert(stream);

	if (clear & NCE_STREAM_CLEAR_INPUT)
		stream->in_len = 0;
	if (clear & NCE_STREAM_CLEAR_OUTPUT)
		stream->out_len = 0;

	nce_stream_io_set(EV_A_ stream, 0);
}

void
nce_stream_drain(EV_P_ struct nce_stream *stream, size_t count)
{
	assert(stream);

	if (count > stream->in_len)
		count = stream->in_len;

	nce_stream_drain_buf(stream->in, &stream->in_len, stream->in_cap, count);
	nce_stream_io_set(EV_A_ stream, 0);
}

static inline int
nce_stream_io(EV_P_ struct nce_stream *stream, int revents)
{
	int rc;

	if ((revents & EV_READ) && (rc = nce_stream_io_read(EV_A_ stream)) < 0)
		return rc;
	if ((revents & EV_WRITE) && (rc = nce_stream_io_write(EV_A_ stream)) < 0)
		return rc;

	return 0;
}

int
nce_stream_wait(EV_P_ struct nce_stream *stream)
{
	assert(stream);

	int revents;

	if (stream->fd < 0)
		return -EBADF;

	/* Yield until at least one condition is true. */
	revents = nce_io_wait(&stream->io_fd);

	return nce_stream_io(EV_A_ stream, revents);
}

int
nce_stream_ready(EV_P_ struct nce_stream *stream)
{
	assert(stream);

	int rc = 0, revents;

	if ((revents = nce_io_ready(&stream->io_fd)))
		rc = nce_stream_io(EV_A_ stream, revents);
	else
		rc = -EAGAIN;

	return rc;
}

int
nce_stream_flush(EV_P_ struct nce_stream *stream)
{
	assert(stream);

	int rc = 0;

	while (stream->out_len > 0 && rc == 0)
		rc = nce_stream_wait(EV_A_ stream);

	return rc;
}

int
nce_stream_coro_spawn(EV_P_ struct nce_stream_coro *sco)
{
	assert(sco);

	int rc;

	if (!(sco->coro.flags & NCE_INACTIVE))
		nce_stream_start(EV_A_ &sco->stream);

	if ((rc = nce_coro_create(EV_A_ &sco->coro)) < 0)
		nce_stream_stop(EV_A_ &sco->stream);
	else
		nce_coro_resume(&sco->coro);

	return rc;
}

void
nce_stream_coro_terminate(EV_P_ struct nce_coro *self)
{
	struct nce_stream_coro *sco = NCE_STREAM_CORO(self, coro);

	nce_stream_stop(EV_A_ &sco->stream);
}

void
nce_stream_coro_destroy(EV_P_ struct nce_stream_coro *sco)
{
	assert(sco);

	nce_stream_stop(EV_A_ &sco->stream);
	nce_coro_destroy(&sco->coro);
}
