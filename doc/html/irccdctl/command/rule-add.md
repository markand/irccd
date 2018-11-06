---
title: rule-add
guide: yes
---

# rule-add

Add a new rule to irccd.

If no index is specified, the rule is added to the end.

## Usage

````nohighlight
$ irccdctl rule-add [options] accept|drop
````

Available options:

  - **-c, --add-channel**: match a channel
  - **-e, --add-event**: match an event
  - **-i, --index**: rule position
  - **-p, --add-plugin**: match a plugin
  - **-s, --add-server**: match a server

## Example

````nohighlight
$ irccdctl rule-add -p hangman drop
$ irccdctl rule-add -s localhost -c #games -p hangman accept
````
