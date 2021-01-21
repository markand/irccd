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

/* Server limits. */
#define IRC_NICKNAME_LEN        32      /* Nickname. */
#define IRC_USERNAME_LEN        32      /* User name. */
#define IRC_REALNAME_LEN        64      /* Real name. */
#define IRC_CHANNEL_LEN         64      /* Channel name. */
#define IRC_PASSWORD_LEN        64      /* Password length. */
#define IRC_CTCPVERSION_LEN     64      /* Custom CTCP version answer. */
#define IRC_USERMODES_LEN       8       /* Number of modes (e.g. ohv). */
#define IRC_CMDCHAR_LEN         4       /* Prefix for plugin commands (e.g. !). */

/* Network limits. */
#define IRC_HOST_LEN            64      /* Hostname length.. */
#define IRC_BUF_LEN             128000  /* Network buffer input/output. */

/* Generic limits. */
#define IRC_ID_LEN              16      /* Plugin/server identifiers. */

/* Rule limits. */
#define IRC_RULE_LEN            1024    /* Space-separated list of values. */

#endif /* !IRCCD_LIMITS_H */
