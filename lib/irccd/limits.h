/*
 * limits.h -- irccd limits
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_LIMITS_H
#define IRCCD_LIMITS_H

/*
 * Those were IRC limits but not strictly following RFC 1459 because lots of
 * servers allow higher limits.
 */
#define IRC_NICKNAME_MAX        32
#define IRC_USERNAME_MAX        32
#define IRC_REALNAME_MAX        64
#define IRC_CHANNEL_MAX         64
#define IRC_PASSWORD_MAX        64
#define IRC_CTCPVERSION_MAX     128
#define IRC_USERMODES_MAX       16

#define IRC_MESSAGE_MAX         512
#define IRC_ARGS_MAX            32

/* Network limits. */
#define IRC_HOST_MAX            32
#define IRC_BUF_MAX             128000

/* Types limits. */
#define IRC_NAME_MAX            16
#define IRC_COMMANDCHAR_MAX     8

#endif /* !IRCCD_LIMITS_H */
