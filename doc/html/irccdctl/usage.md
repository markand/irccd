---
header: Irccdctl usage and options
guide: yes
---

# Usage

Usage of irccdctl.

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
