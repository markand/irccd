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

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <irccd/compat.h>
#include <irccd/irccd.h>
#include <irccd/log.h>
#include <irccd/rule.h>
#include <irccd/server.h>
#include <irccd/util.h>

#include "transport.h"

extern FILE *yyin;

struct pair {
	char *key;
	char *value;
	SLIST_ENTRY(pair) link;
};

SLIST_HEAD(pair_list, pair);

struct string {
	char *value;
	SLIST_ENTRY(string) link;
};

SLIST_HEAD(string_list, string);

struct rule_params {
	struct string_list *servers;
	struct string_list *channels;
	struct string_list *origins;
	struct string_list *plugins;
	struct string_list *events;
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
	struct string_list *channels;
};

struct plugin_params {
	char *location;
	struct pair_list *config;
	struct pair_list *paths;
	struct pair_list *templates;
};

static void
pair_list_finish(struct pair_list *list)
{
	struct pair *pair, *tmp;

	if (!list)
		return;

	SLIST_FOREACH_SAFE(pair, list, link, tmp) {
		free(pair->key);
		free(pair->value);
		free(pair);
	}

	free(list);
}

static void
string_list_finish(struct string_list *list)
{
	struct string *string, *tmp;

	if (!list)
		return;

	SLIST_FOREACH_SAFE(string, list, link, tmp) {
		free(string->value);
		free(string);
	}

	free(list);
}

int
yylex(void);

void
yyerror(const char *);

%}

%union {
	int ival;
	char *sval;

	struct pair *pair;
	struct pair_list *pair_list;

	struct string *string;
	struct string_list *string_list;

	struct server_params *server;
	struct plugin_params *plugin;
	struct rule_params *rule;
};

%token <ival> T_NUMBER T_LOG_VERBOSITY T_RULE_ACTION
%token <sval> T_STRING T_LOG_TYPE

%token T_BRACE_CLOSE
%token T_BRACE_OPEN
%token T_CHANNELS
%token T_COMMA
%token T_CONFIG
%token T_EVENTS
%token T_HOOK
%token T_HOSTNAME
%token T_IDENT
%token T_LOCATION
%token T_LOGS
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

%type <ival> log_verbosity
%type <sval> plugin_location
%type <plugin> plugin_params plugin_params_opt
%type <rule> rule_params rule_params_opt
%type <server> server_params

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

logs
	: T_LOGS log_verbosity
	{
		irc_log_set_verbose($2);
	}
	| T_LOGS log_verbosity T_TO T_LOG_TYPE
	{
		irc_log_set_verbose($2);

		if (strcmp($4, "console") == 0)
			irc_log_to_console();
		else if (strcmp($4, "syslog") == 0)
			irc_log_to_syslog();
		else
			errx(1, "missing log file path");

		free($4);
	}
	| T_LOGS log_verbosity T_TO T_LOG_TYPE T_STRING
	{
		irc_log_to_file($4);
		free($4);
	}
	;

transport
	: T_TRANSPORT T_TO T_STRING
	{
		transport_bind($3);
		free($3);
	}
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
		$$ = irc_util_calloc(1, sizeof (*$$));
		SLIST_INIT($$);
		SLIST_INSERT_HEAD($$, $1, link);
	}
	| string T_COMMA string_list
	{
		$$ = $3;
		SLIST_INSERT_HEAD($$, $1, link);
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
		$$ = irc_util_calloc(1, sizeof (*$$));
		SLIST_INIT($$);
		SLIST_INSERT_HEAD($$, $1, link);
	}
	| pair T_SEMICOLON pair_list
	{
		$$ = $3;
		SLIST_INSERT_HEAD($$, $1, link);
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
			if ($3->servers)
				SLIST_FOREACH(string, $3->servers, link)
					irc_rule_add(rule->servers, string->value);
			if ($3->channels)
				SLIST_FOREACH(string, $3->channels, link)
					irc_rule_add(rule->channels, string->value);
			if ($3->origins)
				SLIST_FOREACH(string, $3->origins, link)
					irc_rule_add(rule->origins, string->value);
			if ($3->plugins)
				SLIST_FOREACH(string, $3->plugins, link)
					irc_rule_add(rule->plugins, string->value);
			if ($3->events)
				SLIST_FOREACH(string, $3->events, link)
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

		SLIST_FOREACH(s, $2, link) {
			if (strcmp(s->value, "AUTO-REJOIN") == 0)
				$$->flags |= IRC_SERVER_FLAGS_AUTO_REJOIN;
			else if (strcmp(s->value, "JOIN-INVITE") == 0)
				$$->flags |= IRC_SERVER_FLAGS_JOIN_INVITE;
			else if (strcmp(s->value, "AUTO-RECONNECT") == 0)
				$$->flags |= IRC_SERVER_FLAGS_AUTO_RECO;
			else
				errx(1, "invalid server option: %s", s->value);
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

		if (!$4->hostname)
			errx(1, "missing server hostname");
		if (!$4->nickname)
			errx(1, "missing server nickname");
		if (!$4->username)
			errx(1, "missing server username");
		if (!$4->realname)
			errx(1, "missing server realname");

		s = irc_server_new($2, $4->nickname, $4->username, $4->realname,
			$4->hostname, $4->port);

		if ($4->channels) {
			SLIST_FOREACH(str, $4->channels, link) {
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

		if (!(p = irc_bot_plugin_find($2, location)))
			goto cleanup;

		if ($3 && $3->templates)
			SLIST_FOREACH(kv, $3->templates, link)
				irc_plugin_set_template(p, kv->key, kv->value);
		if ($3 && $3->config)
			SLIST_FOREACH(kv, $3->config, link)
				irc_plugin_set_option(p, kv->key, kv->value);
		if ($3 && $3->paths)
			SLIST_FOREACH(kv, $3->paths, link)
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
		irc_bot_hook_add(irc_hook_new($2, $4));

		free($2);
		free($4);
	}
	;

%%

void
yyerror(const char *err)
{
	errx(1, "%s", err);
}

void
config_open(const char *path)
{
	if (!(yyin = fopen(path, "r")))
		err(1, "%s", path);

	yyparse();
	fclose(yyin);
}
