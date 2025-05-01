/*
 * hook.c -- irccd hooks
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

#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <utlist.h>

#include "event.h"
#include "hook.h"
#include "irccd.h"
#include "log.h"
#include "server.h"
#include "util.h"

static char **
alloc(const struct irc_hook *h, size_t n, ...)
{
	char **ret;
	va_list ap;

	ret = irc_util_calloc(n + 2, sizeof (*ret));
	ret[0] = (char *)h->path;

	va_start(ap, n);

	for (size_t i = 0; i < n; ++i)
		ret[i + 1] = va_arg(ap, char *);

	va_end(ap);

	return ret;
}

static char **
alloc_mode(const struct irc_hook *h, const struct irc_event *ev)
{
	size_t n = 6;
	char **ret;

	/* Ret contains now 6 values. */
	ret = alloc(h, 5, "onMode", ev->server->name, ev->mode.origin,
	    ev->mode.channel, ev->mode.mode);

	for (char **mode = ev->mode.args; *mode; ++mode) {
		ret = irc_util_reallocarray(ret, n + 1, sizeof (char *));
		ret[n++] = *mode;
	};

	ret = irc_util_reallocarray(ret, n + 1, sizeof (char *));
	ret[n] = NULL;

	return ret;
}

static char **
make_args(const struct irc_hook *h, const struct irc_event *ev)
{
	char **ret;

	switch (ev->type) {
	case IRC_EVENT_CONNECT:
		ret = alloc(h, 2, "onConnect", ev->server->name);
		break;
	case IRC_EVENT_DISCONNECT:
		ret = alloc(h, 2, "onDisconnect", ev->server->name);
		break;
	case IRC_EVENT_INVITE:
		ret = alloc(h, 4, "onInvite", ev->server->name, ev->invite.origin,
		    ev->invite.channel);
		break;
	case IRC_EVENT_JOIN:
		ret = alloc(h, 4, "onJoin", ev->server->name, ev->join.origin,
		    ev->join.channel);
		break;
	case IRC_EVENT_KICK:
		ret = alloc(h, 6, "onKick", ev->server->name, ev->kick.origin,
		    ev->kick.channel, ev->kick.target, ev->kick.reason);
		break;
	case IRC_EVENT_ME:
		ret = alloc(h, 5, "onMe", ev->server->name, ev->message.origin,
		    ev->message.channel, ev->message.message);
		break;
	case IRC_EVENT_MESSAGE:
		ret = alloc(h, 5, "onMessage", ev->server->name, ev->message.origin,
		    ev->message.channel, ev->message.message);
		break;
	case IRC_EVENT_MODE:
		ret = alloc_mode(h, ev);
		break;
	case IRC_EVENT_NICK:
		ret = alloc(h, 4, "onNick", ev->server->name, ev->nick.origin,
		    ev->nick.nickname);
		break;
	case IRC_EVENT_NOTICE:
		ret = alloc(h, 5, "onNotice", ev->server->name, ev->notice.origin,
		    ev->notice.channel, ev->notice.notice);
		break;
	case IRC_EVENT_PART:
		ret = alloc(h, 5, "onPart", ev->server->name, ev->part.origin,
		    ev->part.channel, ev->part.reason);
		break;
	case IRC_EVENT_TOPIC:
		ret = alloc(h, 5, "onTopic", ev->server->name, ev->topic.origin,
		    ev->topic.channel, ev->topic.topic);
		break;
	default:
		return NULL;
	}

	return ret;
}

static void
child_cb(struct ev_loop *loop, struct ev_child *self, int revents)
{
	(void)revents;

	struct irc_hook_child *child;
	struct irc_hook *parent;

	child = IRC_CONTAINER_OF(self, struct irc_hook_child, child);
	parent = child->parent;

	if (WIFEXITED(self->rstatus))
		irc_log_debug("hook %s: exited with code %d", parent->name, WEXITSTATUS(self->rstatus));
	else if (WIFSIGNALED(self->rstatus))
		irc_log_debug("hook %s: terminated on signal %d", parent->name, WTERMSIG(self->rstatus));

	ev_child_stop(loop, self);
	ev_timer_stop(loop, &child->timer);

	LL_DELETE(child->parent->children, child);
	free(child);
}

static void
append(struct irc_hook *h, pid_t pid)
{
	struct irc_hook_child *child;

	child = irc_util_calloc(1, sizeof (*child));
	child->pid = pid;
	child->parent = h;

	ev_child_init(&child->child, child_cb, child->pid, 0);
	ev_child_start(irc_bot_loop(), &child->child);

	LL_APPEND(h->children, child);
}

static void
timer_cb(struct ev_loop *loop, struct ev_timer *self, int revents)
{
	(void)loop;
	(void)revents;

	struct irc_hook_child *child;

	child = IRC_UTIL_CONTAINER_OF(self, struct irc_hook_child, timer);

	irc_log_warn("hook: would not die, sending SIGKILL");
	kill(child->pid, SIGKILL);
}

static inline int
child_exists(const struct irc_hook *hook, const struct irc_hook_child *child)
{
	const struct irc_hook_child *it;

	LL_FOREACH(hook->children, it)
		if (it == child)
			return 1;

	return 0;
}

static void
stop(struct irc_hook *hook, struct irc_hook_child *child)
{
	irc_log_debug("hook %s: stopping active children", hook->name);

	/* Start a timer in case it wouldn't stop on SIGTERM. */
	ev_timer_init(&child->timer, timer_cb, 5.0, 0.0);
	ev_timer_start(irc_bot_loop(), &child->timer);

	/*
	 * Note: the child_cb removes the child from the parent hook so we have
	 * to check if the hook is still present rather than dereferencing the
	 * pointer itself.
	 */
	while (child_exists(hook, child))
		ev_run(irc_bot_loop(), EVRUN_ONCE);
}

struct irc_hook *
irc_hook_new(const char *name, const char *path)
{
	assert(name);
	assert(path);

	struct irc_hook *h;

	h = irc_util_malloc(sizeof (*h));
	h->name = irc_util_strdup(name);
	h->path = irc_util_strdup(path);

	return h;
}

void
irc_hook_invoke(struct irc_hook *h, const struct irc_event *ev)
{
	assert(h);
	assert(ev);

	char **args;
	pid_t pid;

	if (!(args = make_args(h, ev)))
		return;

	switch ((pid = fork())) {
	case -1:
		irc_log_warn("hook %s: %s", h->name, strerror(errno));
		break;
	case 0:
		execv(h->path, args);
		irc_log_warn("hook %s: %s", h->name, strerror(errno));
		exit(1);
		break;
	default:
		/*
		 * Append a hook child handle and wait for it to terminate
		 * asynchronously.
		 */
		append(h, pid);
		break;
	}

	free(args);
}

void
irc_hook_free(struct irc_hook *h)
{
	assert(h);

	struct irc_hook_child *child, *tmp;

	LL_FOREACH_SAFE(h->children, child, tmp)
		stop(h, child);

	free(h->name);
	free(h->path);
	free(h);
}
