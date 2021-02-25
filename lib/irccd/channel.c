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
#include "compat.h"
#include "util.h"

struct irc_channel *
irc_channel_new(const char *name, const char *password, int joined)
{
	assert(name);

	struct irc_channel *ch;

	ch = irc_util_calloc(1, sizeof (*ch));
	ch->joined = joined;

	strlcpy(ch->name, name, sizeof (ch->name));
	strlcpy(ch->password, password ? password : "", sizeof (ch->password));

	LIST_INIT(&ch->users);

	return ch;
}

void
irc_channel_add(struct irc_channel *ch, const char *nickname, int modes)
{
	assert(ch);
	assert(nickname);

	struct irc_channel_user *user;

	if (irc_channel_find(ch, nickname))
		return;

	user = irc_util_malloc(sizeof (*user));
	user->modes = modes;
	strlcpy(user->nickname, nickname, sizeof (user->nickname));

	LIST_INSERT_HEAD(&ch->users, user, link);
}

struct irc_channel_user *
irc_channel_find(const struct irc_channel *ch, const char *nickname)
{
	struct irc_channel_user *u;

	LIST_FOREACH(u, &ch->users, link)
		if (strcmp(u->nickname, nickname) == 0)
			return u;

	return NULL;
}

void
irc_channel_clear(struct irc_channel *ch)
{
	assert(ch);

	struct irc_channel_user *user, *tmp;

	LIST_FOREACH_SAFE(user, &ch->users, link, tmp)
		free(user);
	LIST_INIT(&ch->users);
}

void
irc_channel_remove(struct irc_channel *ch, const char *nick)
{
	assert(ch);
	assert(nick);

	struct irc_channel_user *user;

	if ((user = irc_channel_find(ch, nick))) {
		LIST_REMOVE(user, link);
		free(user);
	}
}

void
irc_channel_finish(struct irc_channel *ch)
{
	assert(ch);

	irc_channel_clear(ch);
	free(ch);
}
