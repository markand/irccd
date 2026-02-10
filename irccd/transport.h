/*
 * transport.h -- remote command support
 *
 * Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_TRANSPORT_H
#define IRCCD_TRANSPORT_H

/**
 * \file transport.h
 * \brief Remote command support.
 */

/**
 * Open and bind transport for irccdctl and peers.
 *
 * \param path path to the socket (not NULL)
 * \param uid the uid to change owner (or -1 ignore)
 * \param gid the gid to change owner (or -1 ignore)
 * \return 0 on success
 * \return -E<*> on error
 */
int
transport_start(const char *path, long long uid, long long gid);

/**
 * Transmit a message to every connected peer.
 *
 * \param data the string to send (not NULL)
 */
void
transport_broadcast(const char *data);

/**
 * Stop the transport and close all connected peers.
 */
void
transport_stop(void);

#endif /* !IRCCD_TRANSPORT_H */
