/*
 * nce/stream.h -- file descriptor support for nce
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

#ifndef LIBNCE_STREAM_H
#define LIBNCE_STREAM_H

/**
 * \file nce/stream.h
 * \brief File descriptor asynchronous I/O stream.
 *
 * This module facilitates the asynchronous I/O management on top of a file
 * descriptor using libnce's nce_io module.
 *
 * It automatically reads user incoming buffer when file descriptor is readable
 * and also flushes the output buffer when writable.
 *
 * ## Different APIs style
 *
 * Because user may have its own needs with regards to the underlying data
 * stream, several functions are available.
 *
 * Functions in the form ::nce_stream_read, ::nce_stream_write are used to read,
 * write as much as requested but possibly less that requested.
 *
 * Functions in the form ::nce_stream_pull, ::nce_stream_push,
 * ::nce_stream_printf are more high level designed as they do the opposite.
 * They only return success if there is greater or equal data to be extracted or
 * greater or equal data available to the output buffer.
 *
 * ## Close on stop
 *
 * The stream by default only use a file descriptor and does not close it but
 * user can provide ::nce_stream_ops::close function and set ::nce_stream::close
 * to automatically close the file descriptor when ::nce_stream_stop is called.
 *
 * \note Because ::nce_stream does not know the file descriptor status, calling
 *       ::nce_stream_stop multiple times is safe as it's no-op once inactive
 *       but calling ::nce_stream_start without changing the ::nce_stream::fd
 *       is undefined behavior.
 */

#include <sys/types.h>
#include <assert.h>
#include <stdarg.h>
#include <stddef.h>

#include <nce/io.h>

struct nce_stream;
struct nce_stream_ops;

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_stream.
 */
#define NCE_STREAM(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_stream, Field))

/**
 * Convenient ::NCE_CONTAINER_OF macro for ::nce_stream_coro.
 */
#define NCE_STREAM_CORO(Ptr, Field) \
	(NCE_CONTAINER_OF(Ptr, struct nce_stream_coro, Field))

/**
 * \enum nce_stream_clear
 * \brief Stream clear flags.
 */
enum nce_stream_clear {
	/**
	 * Empty the input buffer.
	 */
	NCE_STREAM_CLEAR_INPUT = (1 << 0),

	/**
	 * Empty the output buffer.
	 */
	NCE_STREAM_CLEAR_OUTPUT = (1 << 1),

	/**
	 * Empty the input and output buffers.
	 */
	NCE_STREAM_CLEAR = NCE_STREAM_CLEAR_INPUT | NCE_STREAM_CLEAR_OUTPUT
};

/**
 * \struct nce_stream_ops
 * \brief Functions table for read/write.
 *
 * This functions table can be used to provide custom read/write functions when
 * the underlying file descriptor becomes readable or writable.
 */
struct nce_stream_ops {
	/**
	 * Attempt to receive data from the underlying file descriptor.
	 *
	 * \param data the destination buffer
	 * \param len the maximum number of bytes the function can read into
	 * \return 0 if no data is available
	 * \return >0 if some data has been received
	 * \return -EPIPE if the file is considered closed
	 * \return -E<*> other errors happened
	 */
	ssize_t (*read)(EV_P_ struct nce_stream *self, void *data, size_t len);

	/**
	 * Attempt to write to the underlying file descriptor.
	 *
	 * \param data the source buffer
	 * \param len the number of bytes the sources consists of
	 * \return 0 if unable to write data yet
	 * \return >0 if some data has been sent
	 * \return -EPIPE if the file is considered closed
	 * \return -E<*> other errors happened
	 */
	ssize_t (*write)(EV_P_ struct nce_stream *self, const void *data, size_t len);

	/**
	 * Function used to close the ::nstream::fd when calling
	 * ::nce_stream_stop with ::nce_stream::close option set.
	 */
	void (*close)(EV_P_ struct nce_stream *self);
};

/**
 * \brief Default functions table for regular files.
 */
extern const struct nce_stream_ops nce_stream_ops_regular;

/**
 * \brief Default functions table for socket files.
 */
extern const struct nce_stream_ops nce_stream_ops_socket;

/**
 * \struct nce_stream
 * \brief File descriptor support for nce.
 */
struct nce_stream {
	/**
	 * (init)
	 *
	 * Functions table to use for read/write operation.
	 */
	const struct nce_stream_ops *ops;

	/**
	 * (init)
	 *
	 * File descriptor to use for I/O operations.
	 */
	int fd;

	/**
	 * (optional)
	 *
	 * Input buffer.
	 */
	unsigned char *in;

	/**
	 * (optional)
	 *
	 * Capacity of input buffer.
	 */
	size_t in_cap;

	/**
	 * (read-only)
	 *
	 * Input buffer length.
	 */
	size_t in_len;

	/**
	 * (optional)
	 *
	 * Output buffer.
	 */
	unsigned char *out;

	/**
	 * (optional)
	 *
	 * Capacity of output buffer.
	 */
	size_t out_cap;

	/**
	 * (read-only)
	 *
	 * Output buffer length.
	 */
	size_t out_len;

	/**
	 * (optional)
	 *
	 * If non-zero, call to ::nce_stream_stop will close the file descriptor
	 * using the ::nce_stream_ops::close function.
	 */
	int close;

	int in_dyn;             /* !0 if in is allocated by stream */
	int out_dyn;            /* !0 if out is allocated by stream */
	struct nce_io io_fd;    /* file descriptor watcher */
};

/**
 * \struct nce_stream_coro
 * \brief Convenient coroutine coupled with a stream.
 */
struct nce_stream_coro {
	/**
	 * (read-write)
	 *
	 * Underlying stream to use.
	 */
	struct nce_stream stream;

	/**
	 * (read-write)
	 *
	 * Coroutine attached to this watcher.
	 */
	struct nce_coro coro;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Start the stream I/O watcher.
 */
void
nce_stream_start(EV_P_ struct nce_stream *stream);

/**
 * Indicate if the watcher is active.
 *
 * \return non-zero if the watcher was started
 * \sa ::nce_io_start
 */
int
nce_stream_active(const struct nce_stream *stream);

/**
 * Stop the stream watcher and clear internal buffers.
 */
void
nce_stream_stop(EV_P_ struct nce_stream *stream);

/**
 * Read up to length bytes from the incoming buffer, returning maybe less than
 * requested.
 *
 * The number of bytes that have been written in data will be removed from the
 * incoming buffer.
 *
 * \sa ::nce_stream_pull
 * \pre data != NULL
 * \param data the data to fill
 * \param length the maximum number of bytes that can be written in data
 * \return -EBADF is stream is invalid
 * \return the number of bytes added into data
 */
ssize_t
nce_stream_read(EV_P_ struct nce_stream *stream, void *data, size_t length);

/**
 * Queue as much as possible data into the output buffer, returning maybe less
 * than length.
 *
 * \pre data != NULL
 * \param data the data to append
 * \param length the data length to append
 * \return -EBADF is stream is invalid
 * \return -ENOBUFS output buffer is already full
 * \return the number of bytes written in outgoing buffer
 */
int
nce_stream_write(EV_P_ struct nce_stream *stream, const void *data, size_t length);

/**
 * Analogous function to ::nce_stream_read except that this function will read
 * exactly the number of bytes requested.
 *
 * On success, the number of bytes is removed from the incoming buffer and if
 * the incoming buffer was greater than requested bytes, the remaining bytes
 * stays in the incoming buffer.
 *
 * \pre data != NULL
 * \param data the data to fill
 * \param length the maximum number of bytes that can be written in data
 * \return -EBADF is stream is invalid
 * \return -EAGAIN if the requested length is unavailable
 * \return the number of bytes added into data
 */
ssize_t
nce_stream_pull(EV_P_ struct nce_stream *stream, void *data, size_t length);

/**
 * Analogous function to ::nce_stream_write except that this function will write
 * exactly the number of bytes requested.
 *
 * \pre data != NULL
 * \param data the data to append
 * \param length the data length to append
 * \return -EBADF is stream is invalid
 * \return -ENOBUFS is data is larger than current stream capacity
 */
ssize_t
nce_stream_push(EV_P_ struct nce_stream *stream, const void *data, size_t length);

/**
 * Convenient function to push a message using printf(3) format style.
 *
 * Despite its name, this function does not append a trailing NUL terminator
 * into the output buffer.
 *
 * \pre fmt != NULL
 * \param fmt the format string
 * \return -ENOMEM if unable to allocate temporary string
 * \return a value from ::nce_stream_push
 */
#if EV_MULTIPLICITY
__attribute__ ((format(printf, 3, 4)))
#else
__attribute__ ((format(printf, 2, 3)))
#endif
int
nce_stream_printf(EV_P_ struct nce_stream *stream, const char *fmt, ...);

/**
 * Same as ::nce_stream_printf with va_list
 */
#if EV_MULTIPLICITY
__attribute__ ((format(printf, 3, 0)))
#else
__attribute__ ((format(printf, 2, 0)))
#endif
int
nce_stream_vprintf(EV_P_ struct nce_stream *stream, const char *fmt, va_list ap);

/**
 * Clear input and/or output buffers.
 *
 * \param clear bitmask flags
 */
void
nce_stream_clear(EV_P_ struct nce_stream *stream, enum nce_stream_clear clear);

/**
 * Manually discard specific amount of incoming bytes.
 *
 * \param count the number of bytes to discard
 */
void
nce_stream_drain(EV_P_ struct nce_stream *stream, size_t count);

/**
 * Wait until stream gets read/write activity.
 *
 * This function yields until read or write events appears (can be both).
 *
 * \return 0 on success
 * \return -EPIPE if the file is considered closed
 * \return -E<*> other errors happened
 */
int
nce_stream_wait(EV_P_ struct nce_stream *stream);

/**
 * Analogous function to ::nce_stream_wait, this function will only do I/O
 * operations if the stream is currently ready for.
 *
 * This function does not **yield**.
 *
 * \return 0 if the stream was ready and I/O ops were successful
 * \return -EAGAIN if the stream is not ready
 * \return -EPIPE if the file is considered closed
 * \return -E<*> other errors happened
 */
int
nce_stream_ready(EV_P_ struct nce_stream *stream);

/**
 * Wait until output buffer is sent to the file descriptor.
 *
 * This function yields until stream output is empty or an error occurs
 *
 * \return 0 on success
 * \return -EPIPE if the file is considered closed
 * \return -E<*> other errors happened
 */
int
nce_stream_flush(EV_P_ struct nce_stream *stream);

/**
 * This function start the stream and the underlying coroutine.
 *
 * \return refer to ::nce_coro_spawn
 */
int
nce_stream_coro_spawn(EV_P_ struct nce_stream_coro *sco);

/**
 * Usable callback function as ::nce_coro::terminate to stop the ::nce_stream
 * when destroying the coroutine.
 */
void
nce_stream_coro_terminate(EV_P_ struct nce_coro *self);

/**
 * Destroy the watcher and its coroutine.
 *
 * The watcher is stopped **before** destroying the coroutine.
 */
void
nce_stream_coro_destroy(EV_P_ struct nce_stream_coro *sco);

#ifdef __cplusplus
}
#endif

#endif /* !LIBSV_STREAM_H */
