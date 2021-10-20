IRC Client Daemon
=================

IRC Client Daemon aka irccd is a full featured IRC bot written in C.

It runs as a daemon waiting for events. It's also possible to connect to more
than one server with the same instance. It is designed in mind to be simple,
fast and easy to use.

Irccd is also able to use optional Javascript plugins to write convenient
plugins with system sandboxing.

Irccd is also controllable via unix sockets or through its dedicated `irccdctl`
utility allowing you to use irccd as a message relayer and such.

Features
--------

- Simple, fast, clean and lightweight,
- Can use Javascript to create plugins,
- Can connect to multiple servers,
- Can be controlled by sockets and irccdctl,
- Extremely well documented,
- Clean and powerful Javascript API,

Documentation
-------------

Lots of resources are available on the
[official website](http://projects.malikania.fr/irccd)

Author
------

The irccd application was written by David Demelier <markand@malikania.fr>.
