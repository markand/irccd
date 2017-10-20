---
title: rule-edit
guide: yes
---

# rule-edit

Edit an existing rule in irccd.

All options can be specified multiple times.

Available options:

  - **a, --action**: set action
  - **c, --add-channel**: match a channel
  - **C, --remove-channel**: remove a channel
  - **e, --add-event**: match an event
  - **E, --remove-event**: remove an event
  - **p, --add-plugin**: match a plugin
  - **P, --add-plugin**: remove a plugin
  - **s, --add-server**: match a server
  - **S, --remove-server**: remove a server

## Usage

```nohighlight
usage: irccdctl rule-edit [options] index
```

## Example

```nohighlight
$ irccdctl rule-edit -p hangman 0
$ irccdctl rule-edit -S localhost -c #games -p hangman 1
```
