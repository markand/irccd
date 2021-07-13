/*
 * conf.y -- config parser
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

%{

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utlist.h>

#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/rule.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "transport.h"

extern FILE *yyin;
extern int yylineno;

struct pair {
	struct pair *next;
	char *key;
	char *value;
};

struct string {
	struct string *next;
	char *value;
};

struct rule_params {
	struct string *servers;
	struct string *channels;
	struct string *origins;
	struct string *plugins;
	struct string *events;
};

struct transport_params {
	uid_t uid;
	gid_t gid;
};

struct server_params {
	char *hostname;
	char *password;
	int port;
	int flags;
	char *nickname;
	char *username;
	char *realname;
	char *prefix;
	struct string *channels;
};

struct plugin_params {
	char *location;
	struct pair *config;
	struct pair *paths;
	struct pair *templates;
};

static void
pair_list_finish(struct pair *list)
{
	struct pair *pair, *tmp;

	if (!list)
		return;

	LL_FOREACH_SAFE(list, pair, tmp) {
		free(pair->key);
		free(pair->value);
		free(pair);
	}
}

static void
string_list_finish(struct string *list)
{
	struct string *string, *tmp;

	if (!list)
		return;

	LL_FOREACH_SAFE(list, string, tmp) {
		free(string->value);
		free(string);
	}
}

static char confpath[PATH_MAX];

int
yylex(void);

void
yyerror(const char *);

%}

%union {
	int ival;
	char *sval;
	uid_t uid;
	gid_t gid;

	struct pair *pair;
	struct pair *pair_list;

	struct string *string;
	struct string *string_list;

	struct server_params *server;
	struct plugin_params *plugin;
	struct rule_params *rule;
	struct transport_params *tpt;
};

%token <ival> T_NUMBER T_LOG_VERBOSITY T_RULE_ACTION
%token <sval> T_STRING T_LOG_TYPE

%token T_BRACE_CLOSE
%token T_BRACE_OPEN
%token T_CHANNELS
%token T_COMMA
%token T_CONFIG
%token T_EVENTS
%token T_GID
%token T_HOOK
%token T_HOSTNAME
%token T_IDENT
%token T_LOCATION
%token T_LOGS
%token T_LOG_TEMPLATE
%token T_OPTIONS
%token T_ORIGINS
%token T_PASSWORD
%token T_PATHS
%token T_PLUGIN
%token T_PLUGINS
%token T_PREFIX
%token T_PORT
%token T_RULE
%token T_SEMICOLON
%token T_SERVER
%token T_SERVERS
%token T_SSL
%token T_TEMPLATES
%token T_TO
%token T_TRANSPORT
%token T_UID
%token T_WITH

%type <ival> log_verbosity
%type <sval> plugin_location log_template
%type <plugin> plugin_params plugin_params_opt
%type <rule> rule_params rule_params_opt
%type <server> server_params
%type <uid> transport_params_uid
%type <gid> transport_params_gid
%type <tpt> transport_params

%type <pair> pair
%type <pair_list> pair_list plugin_templates plugin_config plugin_paths

%type <string_list> string_list
%type <string> string

%%

config_list
	: config
	| config_list config
	;

config
	: server
	| logs
	| transport
	| rule
	| plugin
	| hook
	;

string
	: T_STRING
	{
		$$ = irc_util_calloc(1, sizeof (*$$));
		$$->value = $1;
	}

string_list
	: string
	{
		$$ = $1;
	}
	| string T_COMMA string_list
	{
		LL_PREPEND($3, $1);
		$$ = $3;
	}
	;

pair
	: T_STRING T_STRING
	{
		$$ = irc_util_calloc(1, sizeof (*$$));
		$$->key = $1;
		$$->value = $2;
	}
	;

pair_list
	: pair T_SEMICOLON
	{
		$$ = $1;
	}
	| pair T_SEMICOLON pair_list
	{
		LL_PREPEND($3, $1);
		$$ = $3;
	}
	;

log_verbosity
	: T_LOG_VERBOSITY
	{
		$$ = $1;
	}
	|
	{
		$$ = 0;
	}
	;

log_template
	: T_LOG_TEMPLATE T_STRING
	{
		$$ = $2;
	}
	|
	{
		$$ = NULL;
	}
	;

logs
	: T_LOGS log_verbosity log_template
	{
		irc_log_set_verbose($2);
		irc_log_set_template($3);
		free($3);
	}
	| T_LOGS log_verbosity log_template T_TO T_LOG_TYPE
	{
		irc_log_set_verbose($2);
		irc_log_set_template($3);

		if (strcmp($5, "console") == 0)
			irc_log_to_console();
		else if (strcmp($5, "syslog") == 0)
			irc_log_to_syslog();
		else
			irc_util_die("missing log file path\n");

		free($3);
		free($5);
	}
	| T_LOGS log_verbosity log_template T_TO T_LOG_TYPE T_STRING
	{
		irc_log_to_file($6);
		irc_log_set_template($3);
		free($3);
		free($6);
	}
	;

transport_params_uid
	: T_UID T_NUMBER
	{
		$$ = (uid_t)$2;
	}
	| T_UID T_STRING
	{
		struct passwd *pwd;

		if (!(pwd = getpwnam($2)))
			irc_util_die("invalid uid: %s\n", $2);

		free($2);
		$$ = pwd->pw_uid;
	}
	;

transport_params_gid
	: T_GID T_NUMBER
	{
		$$ = (gid_t)$2;
	}
	| T_GID T_STRING
	{
		struct group *grp;

		if (!(grp = getgrnam($2)))
			irc_util_die("invalid uid: %s\n", $2);

		free($2);
		$$ = grp->gr_gid;
	}
	;

transport_params
	: T_WITH transport_params_uid transport_params_gid
	{
		$$ = irc_util_malloc(sizeof (*$$));
		$$->uid = $2;
		$$->gid = $3;
	}
	|
	{
		$$ = NULL;
	}
	;

transport
	: T_TRANSPORT T_TO T_STRING transport_params
	{
		if ($4)
			transport_bindp($3, $4->uid, $4->gid);
		else
			transport_bind($3);

		free($3);
		free($4);
	}
	;

rule_params
	: T_SERVERS string_list T_SEMICOLON rule_params
	{
		$$ = $4;
		$$->servers = $2;
	}
	| T_CHANNELS string_list T_SEMICOLON rule_params
	{
		$$ = $4;
		$$->channels = $2;
	}
	| T_ORIGINS string_list T_SEMICOLON rule_params
	{
		$$ = $4;
		$$->origins = $2;
	}
	| T_EVENTS string_list T_SEMICOLON rule_params
	{
		$$ = $4;
		$$->events = $2;
	}
	| T_PLUGINS string_list T_SEMICOLON rule_params
	{
		$$ = $4;
		$$->plugins = $2;
	}
	|
	{
		$$ = irc_util_calloc(1, sizeof (*$$));
	}
	;

rule_params_opt
	: T_BRACE_OPEN rule_params T_BRACE_CLOSE
	{
		$$ = $2;
	}
	|
	{
		$$ = NULL;
	}
	;

rule
	: T_RULE T_RULE_ACTION rule_params_opt
	{
		struct irc_rule *rule;
		struct string *string;

		rule = irc_rule_new($2);

		if ($3) {
			LL_FOREACH($3->servers, string)
				irc_rule_add(rule->servers, string->value);
			LL_FOREACH($3->channels, string)
				irc_rule_add(rule->channels, string->value);
			LL_FOREACH($3->origins, string)
				irc_rule_add(rule->origins, string->value);
			LL_FOREACH($3->plugins, string)
				irc_rule_add(rule->plugins, string->value);
			LL_FOREACH($3->events, string)
				irc_rule_add(rule->events, string->value);

			string_list_finish($3->servers);
			string_list_finish($3->channels);
			string_list_finish($3->origins);
			string_list_finish($3->plugins);
			string_list_finish($3->events);
		}

		irc_bot_rule_insert(rule, -1);

		free($3);
	}
	;

server_params
	: T_HOSTNAME T_STRING T_SEMICOLON server_params
	{
		$$ = $4;
		$$->hostname = $2;
	}
	| T_PORT T_NUMBER T_SEMICOLON server_params
	{
		$$ = $4;
		$$->port = $2;
	}
	| T_IDENT T_STRING T_STRING T_STRING T_SEMICOLON server_params
	{
		$$ = $6;
		$$->nickname = $2;
		$$->username = $3;
		$$->realname = $4;
	}
	| T_CHANNELS string_list T_SEMICOLON server_params
	{
		$$ = $4;
		$$->channels = $2;
	}
	| T_SSL T_SEMICOLON server_params
	{
		$$ = $3;
		$$->flags |= IRC_SERVER_FLAGS_SSL;
	}
	| T_PASSWORD T_STRING T_SEMICOLON server_params
	{
		$$ = $4;
		$$->password = $2;
	}
	| T_PREFIX T_STRING server_params
	{
		$$ = $3;
		$$->prefix = $2;
	}
	| T_OPTIONS string_list T_SEMICOLON server_params
	{
		struct string *s;

		$$ = $4;

		LL_FOREACH($2, s) {
			if (strcmp(s->value, "AUTO-REJOIN") == 0)
				$$->flags |= IRC_SERVER_FLAGS_AUTO_REJOIN;
			else if (strcmp(s->value, "JOIN-INVITE") == 0)
				$$->flags |= IRC_SERVER_FLAGS_JOIN_INVITE;
			else if (strcmp(s->value, "AUTO-RECONNECT") == 0)
				$$->flags |= IRC_SERVER_FLAGS_AUTO_RECO;
			else
				irc_util_die("invalid server option: %s\n", s->value);
		}

		string_list_finish($2);
	}
	|
	{
		$$ = irc_util_calloc(1, sizeof (*$$));
	}
	;

server
	: T_SERVER T_STRING T_BRACE_OPEN server_params T_BRACE_CLOSE
	{
		struct irc_server *s;
		struct string *str;
		char *at;

		if (irc_bot_server_get($2))
			irc_util_die("server %s already exists\n", $2);
		if (!$4->hostname)
			irc_util_die("missing server hostname\n");
		if (!$4->nickname)
			irc_util_die("missing server nickname\n");
		if (!$4->username)
			irc_util_die("missing server username\n");
		if (!$4->realname)
			irc_util_die("missing server realname\n");

		s = irc_server_new($2, $4->nickname, $4->username, $4->realname,
			$4->hostname, $4->port);

		if ($4->channels) {
			LL_FOREACH($4->channels, str) {
				if ((at = strchr(str->value, '@'))) {
					*at = 0;
					irc_server_join(s, at + 1, str->value);
				} else
					irc_server_join(s, str->value, NULL);
			}

			string_list_finish($4->channels);
		}

		if ($4->prefix)
			strlcpy(s->prefix, $4->prefix, sizeof (s->prefix));
		if ($4->password)
			strlcpy(s->ident.password, $4->password, sizeof (s->ident.password));

		s->flags = $4->flags;
		irc_bot_server_add(s);

		free($2);
		free($4->hostname);
		free($4->nickname);
		free($4->username);
		free($4->realname);
		free($4->prefix);
		free($4->password);
		free($4);
	}
	;

plugin_location
	: T_LOCATION T_STRING T_SEMICOLON
	{
		$$ = $2;
	}
	;

plugin_templates
	: T_TEMPLATES T_BRACE_OPEN pair_list T_BRACE_CLOSE
	{
		$$ = $3;
	}
	;

plugin_paths
	: T_PATHS T_BRACE_OPEN pair_list T_BRACE_CLOSE
	{
		$$ = $3;
	}
	;

plugin_config
	: T_CONFIG T_BRACE_OPEN pair_list T_BRACE_CLOSE
	{
		$$ = $3;
	}
	;

plugin_params
	: plugin_location plugin_params
	{
		$$ = $2;
		$$->location = $1;
	}
	| plugin_templates plugin_params
	{
		$$ = $2;
		$$->templates = $1;
	}
	| plugin_paths plugin_params
	{
		$$ = $2;
		$$->paths = $1;
	}
	| plugin_config plugin_params
	{
		$$ = $2;
		$$->config = $1;
	}
	|
	{
		$$ = irc_util_calloc(1, sizeof (*$$));
	}
	;

plugin_params_opt
	: T_BRACE_OPEN plugin_params T_BRACE_CLOSE
	{
		$$ = $2;
	}
	|
	{
		$$ = NULL;
	}
	;

plugin
	: T_PLUGIN T_STRING plugin_params_opt
	{
		struct irc_plugin *p;
		struct pair *kv;
		const char *location = $3 ? $3->location : NULL;

		if (irc_bot_plugin_get($2))
			irc_util_die("plugin %s already exists\n", $2);
		if (!(p = irc_bot_plugin_find($2, location)))
			goto cleanup;

		if ($3 && $3->templates)
			LL_FOREACH($3->templates, kv)
				irc_plugin_set_template(p, kv->key, kv->value);
		if ($3 && $3->config)
			LL_FOREACH($3->config, kv)
				irc_plugin_set_option(p, kv->key, kv->value);
		if ($3 && $3->paths)
			LL_FOREACH($3->paths, kv)
				irc_plugin_set_path(p, kv->key, kv->value);

		irc_bot_plugin_add(p);

	cleanup:
		if ($3) {
			free($3->location);
			pair_list_finish($3->templates);
			pair_list_finish($3->config);
			pair_list_finish($3->paths);
		}

		free($2);
		free($3);
	}
	;

hook
	: T_HOOK T_STRING T_TO T_STRING
	{
		if (irc_bot_hook_get($2))
			irc_util_die("hook %s already exists\n", $2);

		irc_bot_hook_add(irc_hook_new($2, $4));

		free($2);
		free($4);
	}
	;

%%

void
yyerror(const char *err)
{
	irc_util_die("%s:%d: %s\n", confpath, yylineno, err);
}

void
config_open(const char *path)
{
	if (!(yyin = fopen(path, "r")))
		irc_util_die("%s: %s\n", path, strerror(errno));

	strlcpy(confpath, path, sizeof (confpath));
	yyparse();
	fclose(yyin);
}
