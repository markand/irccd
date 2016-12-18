---
header: Irccdctl usage and options
guide: yes
---

# Usage

Usage of irccdctl.

## Synopsis

````nohighlight
usage: irccdctl plugin-config plugin [variable] [value]
       irccdctl plugin-info plugin
       irccdctl plugin-list
       irccdctl plugin-load logger
       irccdctl plugin-reload plugin
       irccdctl plugin-unload plugin
       irccdctl server-cmode server channel mode
       irccdctl server-cnotice server channel message
       irccdctl server-connect [options] id host [port]
       irccdctl server-disconnect [server]
       irccdctl server-info server
       irccdctl server-invite server nickname channel
       irccdctl server-join server channel [password]
       irccdctl server-kick server target channel [reason]
       irccdctl server-list
       irccdctl server-me server target message
       irccdctl server-message server target message
       irccdctl server-mode server mode
       irccdctl server-nick server nickname
       irccdctl server-notice server target message
       irccdctl server-part server channel [reason]
       irccdctl server-reconnect [server]
       irccdctl server-topic server channel topic
       irccdctl watch [-f|--format json|native]

````

## Syntax

The general syntax for running an irccdctl command is:

````nohighlight
irccdctl commandname arg1 arg2 arg3 ... argn
````

You can have the online documentation by typing `irccdctl help commandname`.

## Shell escaping issue

Some shells may discard arguments if they begins with a hash. For instance, `bash` will not understand the following
command:

````nohighlight
$ irccdctl server-join localhost #staff
````

Instead, enclose the arguments with quotes

````nohighlight
$ irccdctl server-join localhost "#staff"
````
