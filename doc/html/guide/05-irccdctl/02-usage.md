## Usage

Usage of irccdctl.

### Syntax

The general syntax for running an irccdctl command is:

````
irccdctl commandname arg1 arg2 arg3 ... argn
````

You can have the online documentation by typing `irccdctl help commandname`.

### Shell escaping issue

Some shells may discard arguments if they begins with a hash. For instance, `bash` will not understand the following
command:

````
$ irccdctl server-join localhost #staff
````

Instead, enclose the arguments with quotes

````
$ irccdctl server-join localhost "#staff"
````
