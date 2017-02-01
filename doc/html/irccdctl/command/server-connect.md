---
title: server-connect
guide: yes
---

# server-connect

Connect to a new IRC server.

## Usage

````nohighlight
$ irccdctl server-connect [options] name host port
````

Available options:

- **-c, --command**: specify the command char
- **-n, --nickname**: specify a nickname
- **-r, --realname**: specify a real name
- **-S, --ssl-verify**: verify SSL
- **-s, --ssl**: connect using SSL
- **-u, --username**: specify a user name

## Example

````nohighlight
$ irccdctl server-connect -n jean example irc.example.org
$ irccdctl server-connect --ssl example irc.example.org 6697
````
