/*
 * lex.l -- config lexer
 *
 * Copyright (c) 2013-2022 David Demelier <markand@malikania.fr>
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

%option noyywrap
%option nounput
%option noinput
%option yylineno

%{

#include <sys/types.h>

#include <irccd/limits.h>
#include <irccd/rule.h>

#include "conf.h"

void
yyerror(const char *);

%}

channels        channels
config          config
ctcp            ctcp
events          events
gid             gid
hook            hook
hostname        hostname
ident           ident
location        location
log_template    template
log_type        console|syslog|file
log_verbosity   quiet|verbose
logs            logs
options         options
origins         origins
password        password
paths           paths
plugin          plugin
plugins         plugins
port            port
prefix          prefix
rule            rule
rule_action     accept|drop
server          server
servers         servers
ssl             ssl
templates       templates
to              to
transport       transport
uid             uid
with            with

brace_open      \{
brace_close     \}
comma           ,
semicolon       ;

ws              [ \t\n]+
comment         #.*$
number          [0-9]+
word            [A-Za-z_][^{};,"\r\n\t ]*
string          \"([^\\\"]|\\.)*\"
catchall        .

%%

{ws}                    ;
{comment}               ;

{brace_close}           return T_BRACE_CLOSE;
{brace_open}            return T_BRACE_OPEN;
{channels}              return T_CHANNELS;
{comma}                 return T_COMMA;
{config}                return T_CONFIG;
{ctcp}                  return T_CTCP;
{events}                return T_EVENTS;
{gid}                   return T_GID;
{hook}                  return T_HOOK;
{hostname}              return T_HOSTNAME;
{ident}                 return T_IDENT;
{location}              return T_LOCATION;
{log_template}          return T_LOG_TEMPLATE;
{logs}                  return T_LOGS;
{options}               return T_OPTIONS;
{origins}               return T_ORIGINS;
{password}              return T_PASSWORD;
{paths}                 return T_PATHS;
{plugins}               return T_PLUGINS;
{plugin}                return T_PLUGIN;
{port}                  return T_PORT;
{prefix}                return T_PREFIX;
{semicolon}             return T_SEMICOLON;
{servers}               return T_SERVERS;
{server}                return T_SERVER;
{ssl}                   return T_SSL;
{templates}             return T_TEMPLATES;
{to}                    return T_TO;
{uid}                   return T_UID;
{with}                  return T_WITH;

{log_verbosity}         {
                                if (strcmp(yytext, "quiet") == 0)
                                        yylval.ival = 0;
                                else
                                        yylval.ival = 1;

                                return T_LOG_VERBOSITY;
                        }
{log_type}              {
                                yylval.sval = strdup(yytext);
                                return T_LOG_TYPE;
                        }

{transport}             return T_TRANSPORT;

{rule}                  return T_RULE;
{rule_action}           {
                                if (strcmp(yytext, "accept") == 0)
                                        yylval.ival = IRC_RULE_ACCEPT;
                                else
                                        yylval.ival = IRC_RULE_DROP;

                                return T_RULE_ACTION;
                        }

{word}                  {
                                yylval.sval = strdup(yytext);
                                return T_STRING;
                        }

{string}                {
                                yylval.sval = strdup(yytext + 1);
                                yylval.sval[yyleng - 2] = '\0';
                                return T_STRING;
                        }

{number}                {
                                yylval.ival = atoi(yytext);
                                return T_NUMBER;
                        }

{catchall}              {
                                yyerror("unexpected token");
                                return -1;
                        }

%%
