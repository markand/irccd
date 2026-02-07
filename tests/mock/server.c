#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include <utlist.h>

#include "server.h"

IRC_ATTR_PRINTF(2, 3)
static int
append_out(struct irc_server *s, const char *fmt, ...)
{
	struct mock_server *mock;
	struct mock_server_msg *msg;
	va_list ap;

	mock = IRC_UTIL_CONTAINER_OF(s, struct mock_server, parent);
	msg = irc_util_calloc(1, sizeof (*msg));

	va_start(ap, fmt);
	vasprintf(&msg->line, fmt, ap);
	va_end(ap);

	LL_PREPEND(mock->out, msg);

	return 0;
}

static struct irc_channel *
channels_find(struct irc_server *s, const char *name)
{
	struct irc_channel *ch;

	LL_FOREACH(s->channels, ch)
		if (strcasecmp(ch->name, name) == 0)
			return ch;

	return NULL;
}

struct irc_server *
irc_server_new(const char *name)
{
	struct mock_server *s;

	s = irc_util_calloc(1, sizeof (*s));
	s->parent.name = irc_util_strdup(name);
	s->parent.nickname = "t";
	s->parent.username = "t";
	s->parent.realname = "t";
	s->parent.prefix = "!";

	return &s->parent;
}

void
irc_server_set_hostname(struct irc_server *, const char *)
{
}

void
irc_server_set_flags(struct irc_server *, enum irc_server_flags)
{
}

void
irc_server_set_port(struct irc_server *, unsigned int)
{
}

void
irc_server_set_nickname(struct irc_server *, const char *)
{
}

void
irc_server_set_username(struct irc_server *, const char *)
{
}

void
irc_server_set_realname(struct irc_server *, const char *)
{
}

void
irc_server_set_ctcp(struct irc_server *, const char *, const char *)
{
}

void
irc_server_set_prefix(struct irc_server *, const char *)
{
}

void
irc_server_set_password(struct irc_server *, const char *)
{
}

void
irc_server_connect(struct irc_server *)
{
}

void
irc_server_disconnect(struct irc_server *)
{
}

void
irc_server_reconnect(struct irc_server *)
{
}

const struct irc_channel *
irc_server_channels_find(struct irc_server *, const char *)
{
	return NULL;
}

IRC_ATTR_PRINTF(2, 3)
int
irc_server_send(struct irc_server *, const char *, ...)
{
	return 0;
}

IRC_ATTR_PRINTF(2, 0)
int
irc_server_send_va(struct irc_server *, const char *, va_list)
{
	return 0;
}

int
irc_server_invite(struct irc_server *s, const char *channel, const char *target)
{
	return append_out(s, "invite %s %s", channel, target);
}

int
irc_server_join(struct irc_server *s, const char *name, const char *password)
{
	struct irc_channel *channel;

	channel = irc_channel_new(name, password, IRC_CHANNEL_FLAGS_JOINED);
	LL_APPEND(s->channels, channel);

	return append_out(s, "join %s %s", name, password ? password : "nil");
}

int
irc_server_kick(struct irc_server *s, const char *channel, const char *target, const char *reason)
{
	irc_channel_remove(channels_find(s, channel), target);

	return append_out(s, "kick %s %s %s", channel, target, reason ? reason : "nil");
}

int
irc_server_part(struct irc_server *s, const char *channel, const char *reason)
{
	irc_channel_remove(channels_find(s, channel), s->nickname);

	return append_out(s, "part %s %s", channel, reason ? reason : "nil");
}

int
irc_server_topic(struct irc_server *s, const char *channel, const char *topic)
{
	return append_out(s, "tpoic %s %s", channel, topic);
}

int
irc_server_message(struct irc_server *s, const char *target, const char *message)
{
	return append_out(s, "message %s %s", target, message);
}

int
irc_server_me(struct irc_server *s, const char *target, const char *message)
{
	return append_out(s, "me %s %s", target, message);
}

int
irc_server_mode(struct irc_server *s, const char *target, const char *mode, const char *args)
{
	return append_out(s, "mode %s %s %s", target, mode, args ? args : "nil");
}

int
irc_server_names(struct irc_server *s, const char *channel)
{
	return append_out(s, "names %s", channel);
}

int
irc_server_nick(struct irc_server *s, const char *nickname)
{
	return append_out(s, "nick %s", nickname);
}

int
irc_server_notice(struct irc_server *s, const char *target, const char *message)
{
	return append_out(s, "notice %s %s", target, message);
}

int
irc_server_whois(struct irc_server *s, const char *target)
{
	return append_out(s, "whois %s", target);
}

int
irc_server_strip(const struct irc_server *, const char **)
{
	return 0;
}

void
irc_server_incref(struct irc_server *s)
{
	s->refc++;
}

void
irc_server_decref(struct irc_server *s)
{
	if (!--s->refc)
		mock_server_free(s);
}

/*
 * Clear what has been appended to the output queue.
 */
void
mock_server_clear(struct irc_server *s)
{
	struct mock_server *mock;
	struct mock_server_msg *msg, *tmp;

	mock = IRC_UTIL_CONTAINER_OF(s, struct mock_server, parent);

	LL_FOREACH_SAFE(mock->out, msg, tmp) {
		free(msg->line);
		free(msg);
	}

	mock->out = NULL;
}

void
mock_server_free(struct irc_server *s)
{
	struct mock_server *mock;

	mock = IRC_UTIL_CONTAINER_OF(s, struct mock_server, parent);
	mock_server_clear(s);

	free(mock->parent.name);
	free(mock);
}
