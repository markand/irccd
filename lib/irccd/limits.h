/*
 * limits.h -- irccd limits
 *
 * Copyright (c) 2013-2023 David Demelier <markand@malikania.fr>
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

#include <limits.h>

/* Server limits. */
#define IRC_NICKNAME_LEN        32              /* Nickname. */
#define IRC_USERNAME_LEN        32              /* User name. */
#define IRC_REALNAME_LEN        64              /* Real name. */
#define IRC_CHANNEL_LEN         64              /* Channel name. */
#define IRC_PASSWORD_LEN        64              /* Password length. */
#define IRC_CTCP_LEN            64              /* Custom CTCP version answer. */
#define IRC_USERMODES_LEN       8               /* Number of modes (e.g. ohv). */
#define IRC_CHANTYPES_LEN       8               /* Channel types. */
#define IRC_PREFIX_LEN          4               /* Prefix for plugin commands (e.g. !). */
#define IRC_CHARSET_LEN         16              /* Charset name max. */
#define IRC_CASEMAPPING_LEN     16              /* Maximum case mapping length. */
#define IRC_MESSAGE_LEN         512             /* Official length per message. */
#define IRC_ARGS_MAX            32              /* Own supported number of arguments per message. */

/* Network limits. */
#define IRC_HOST_LEN            64              /* Hostname length.. */
#define IRC_BUF_LEN             128000          /* Network buffer input/output. */

/* Generic limits. */
#define IRC_ID_LEN              16              /* Plugin/server identifiers. */
#define IRC_PATHS_LEN           (PATH_MAX * 8)  /* Colon separated list of paths. */
#define IRC_EXTENSIONS_LEN      32              /* Colon separated list of extensions for plugins. */

/* Rule limits. */
#define IRC_RULE_LEN            1024            /* Space-separated list of values. */

#endif /* !IRCCD_LIMITS_H */
