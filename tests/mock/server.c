#include <stdlib.h>
#include <string.h>

#include <utlist.h>

#include <irccd/server.h>
#include <irccd/util.h>

struct mock_server_msg {
	char *line;
	struct mock_server_msg *next;
};

struct mock_server {
	struct irc_server parent;
	struct mock_server_msg *out;
};

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

static void
mock_free(struct irc_server *s)
{
	struct mock_server *mock;
	struct mock_server_msg *msg, *tmp;

	mock = IRC_UTIL_CONTAINER_OF(s, struct mock_server, parent);

	LL_FOREACH_SAFE(mock->out, msg, tmp) {
		free(msg->line);
		free(msg);
	}

	free(mock->parent.name);
	free(mock);
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
irc_server_set_ident(struct irc_server *s,
                     const char *nickname,
                     const char *username,
                     const char *realname)
{
	(void)s;
	(void)nickname;
	(void)username;
	(void)realname;
}

void
irc_server_set_params(struct irc_server *s,
                      const char *hostname,
                      unsigned int port,
                      enum irc_server_flags flags)
{
	(void)s;
	(void)hostname;
	(void)port;
	(void)flags;
}

void
irc_server_set_ctcp(struct irc_server *s,
                    const char *version,
                    const char *source)
{
	(void)s;
	(void)version;
	(void)source;
}

void
irc_server_set_prefix(struct irc_server *s, const char *prefix)
{
	(void)s;
	(void)prefix;
}

void
irc_server_set_password(struct irc_server *s, const char *password)
{
	(void)s;
	(void)password;
}

void
irc_server_connect(struct irc_server *s)
{
	(void)s;
}

void
irc_server_disconnect(struct irc_server *s)
{
	(void)s;
}

void
irc_server_reconnect(struct irc_server *s)
{
	(void)s;
}

const struct irc_channel *
irc_server_channels_find(struct irc_server *s, const char *name)
{
	(void)s;
	(void)name;

	return NULL;
}

IRC_ATTR_PRINTF(2, 3)
int
irc_server_send(struct irc_server *s, const char *fmt, ...)
{
	(void)s;
	(void)fmt;

	return 0;
}

IRC_ATTR_PRINTF(2, 0)
int
irc_server_send_va(struct irc_server *s, const char *fmt, va_list ap)
{
	(void)s;
	(void)fmt;
	(void)ap;

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
irc_server_strip(const struct irc_server *s, const char **nickname)
{
	(void)s;
	(void)nickname;

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
		mock_free(s);
}
