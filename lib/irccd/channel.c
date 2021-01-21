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

static inline struct irc_channel_user *
find(const struct irc_channel *ch, const char *nickname)
{
	struct irc_channel_user *u;

	LIST_FOREACH(u, &ch->users, link)
		if (strcmp(u->nickname, nickname) == 0)
			return u;

	return NULL;
}

struct irc_channel *
irc_channel_new(const char *name, const char *password, bool joined)
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
irc_channel_add(struct irc_channel *ch, const char *nickname, char mode, char symbol)
{
	assert(ch);
	assert(nickname);

	struct irc_channel_user *user;

	if (find(ch, nickname))
		return;

	user = irc_util_malloc(sizeof (*user));
	user->mode = mode;
	user->symbol = symbol;
	strlcpy(user->nickname, nickname, sizeof (user->nickname));

	LIST_INSERT_HEAD(&ch->users, user, link);
}

void
irc_channel_update(struct irc_channel *ch,
                   const char *nickname,
                   const char *newnickname,
                   char mode,
                   char symbol)
{
	assert(ch);
	assert(nickname);

	struct irc_channel_user *user;

	if ((user = find(ch, nickname))) {
		if (newnickname)
			strlcpy(user->nickname, newnickname, sizeof (user->nickname));
		if (mode != -1 && symbol != -1) {
			user->mode = mode;
			user->symbol = symbol;
		}
	}
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

	if ((user = find(ch, nick)))
		LIST_REMOVE(user, link);
}

void
irc_channel_finish(struct irc_channel *ch)
{
	assert(ch);

	irc_channel_clear(ch);
	free(ch);
}
