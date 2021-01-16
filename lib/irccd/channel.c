/*
 * channel.h -- an IRC server channel
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "channel.h"
#include "util.h"
#include "set.h"

static inline int
cmp(const struct irc_channel_user *u1, const struct irc_channel_user *u2)
{
	return strcmp(u1->nickname, u2->nickname);
}

static inline struct irc_channel_user *
find(const struct irc_channel *ch, const char *nick)
{
	struct irc_channel_user key = {0};

	strlcpy(key.nickname, nick, sizeof (key.nickname));

	return IRC_SET_FIND(ch->users, ch->usersz, &key, cmp);
}

void
irc_channel_add(struct irc_channel *ch, const char *nick, char mode)
{
	assert(ch);
	assert(nick);

	if (find(ch, nick))
		return;

	struct irc_channel_user u = {0};

	strlcpy(u.nickname, nick, sizeof (u.nickname));
	u.mode = mode;

	IRC_SET_ALLOC_PUSH(&ch->users, &ch->usersz, &u, cmp);
}

void
irc_channel_set_user_mode(struct irc_channel *ch, const char *nick, char mode)
{
	assert(ch);
	assert(nick);

	struct irc_channel_user *user;

	if ((user = find(ch, nick)))
		user->mode = mode;
}

void
irc_channel_set_user_nick(struct irc_channel *ch, const char *nick, const char *newnick)
{
	assert(ch);
	assert(nick);
	assert(newnick);

	struct irc_channel_user *user;

	if ((user = find(ch, nick)))
		strlcpy(user->nickname, newnick, sizeof (user->nickname));
}

void
irc_channel_clear(struct irc_channel *ch)
{
	assert(ch);

	free(ch->users);
	ch->users = 0;
}

void
irc_channel_remove(struct irc_channel *ch, const char *nick)
{
	assert(ch);
	assert(nick);

	struct irc_channel_user *user;

	if ((user = find(ch, nick)))
		IRC_SET_ALLOC_REMOVE(&ch->users, &ch->usersz, user);
}

void
irc_channel_finish(struct irc_channel *ch)
{
	assert(ch);

	irc_channel_clear(ch);
	memset(ch, 0, sizeof (*ch));
}
