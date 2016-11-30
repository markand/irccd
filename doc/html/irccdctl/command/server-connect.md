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
$ irccdctl server-connect wanadoo chat.wanadoo.fr 6667
$ irccdctl server-connect -s -S -n "undead" wanadoo chat.wanadoo.fr 6697
````
