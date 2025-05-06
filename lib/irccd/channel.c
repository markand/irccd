/*
 * channel.h -- an IRC server channel
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>

#include <utlist.h>

#include "channel.h"
#include "util.h"

static inline struct irc_channel_user *
find(const struct irc_channel *ch, const char *nickname)
{
	struct irc_channel_user *u;

	LL_FOREACH(ch->users, u)
		if (strcasecmp(u->nickname, nickname) == 0)
			return u;

	return NULL;
}

struct irc_channel *
irc_channel_new(const char *name,
                const char *password,
                enum irc_channel_flags flags)
{
	assert(name);

	struct irc_channel *ch;

	ch = irc_util_calloc(1, sizeof (*ch));
	ch->name = irc_util_strdup(name);
	ch->flags = flags;

	if (password)
		ch->password = irc_util_strdup(password);

	/* Convert to lowercase */
	for (char *c = ch->name; *c; ++c)
		*c = tolower((unsigned char)*c);

	return ch;
}

void
irc_channel_add(struct irc_channel *ch, const char *nickname, int modes)
{
	assert(ch);
	assert(nickname);

	struct irc_channel_user *user;

	if (irc_channel_get(ch, nickname))
		return;

	user = irc_util_calloc(1, sizeof (*user));
	user->nickname = irc_util_strdup(nickname);
	user->modes = modes;

	LL_PREPEND(ch->users, user);
}

const struct irc_channel_user *
irc_channel_get(const struct irc_channel *ch, const char *nickname)
{
	assert(ch);
	assert(nickname);

	return find(ch, nickname);
}

void
irc_channel_set(struct irc_channel *ch, const char *nickname, int modes)
{
	assert(ch);
	assert(nickname);

	struct irc_channel_user *u;

	if ((u = find(ch, nickname)))
		u->modes = modes;
}

void
irc_channel_clear(struct irc_channel *ch)
{
	assert(ch);

	struct irc_channel_user *user, *tmp;

	LL_FOREACH_SAFE(ch->users, user, tmp) {
		free(user->nickname);
		free(user);
	}

	ch->users = NULL;
	ch->flags = IRC_CHANNEL_FLAGS_NONE;
}

size_t
irc_channel_count(const struct irc_channel *ch)
{
	assert(ch);

	const struct irc_channel_user *user;
	size_t rc = 0;

	LL_FOREACH(ch->users, user)
		++rc;

	return rc;
}

void
irc_channel_remove(struct irc_channel *ch, const char *nickname)
{
	assert(ch);
	assert(nickname);

	struct irc_channel_user *user;

	if ((user = find(ch, nickname))) {
		LL_DELETE(ch->users, user);
		free(user->nickname);
		free(user);
	}
}

void
irc_channel_free(struct irc_channel *ch)
{
	if (ch) {
		irc_channel_clear(ch);

		free(ch->name);
		free(ch->password);
		free(ch);
	}
}
