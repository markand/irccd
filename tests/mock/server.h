#ifndef IRCCD_TESTS_MOCK_SERVER_H
#define IRCCD_TESTS_MOCK_SERVER_H

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

void
mock_server_clear(struct irc_server *);

void
mock_server_free(struct irc_server *s);

#endif /* !IRCCD_TESTS_MOCK_SERVER_H */
