---
header: Configuring irccdctl
guide: yes
---

The `irccdctl` utility let you controlling a running `irccd` instance. It uses sockets to perform any operation.

You need to define at least one transport before using `irccdctl`.

# The general section

This section defines the global irccdctl parameters.

The available options:

- **verbose**: (bool) enable verbose message (Optional, default: false).

**Example**

````ini
[general]
verbose = true
````

# The connect section

The section socket permit irccdctl to connect to a specific irccd listener, only one may be defined. Just like
transports you can connect to Unix or internet sockets.

The available options:

  - **type**: (string) connection type: "ip" or "unix".
  - **password**: (string) an authentication password (Optional, default: none).

The options for **ip** type:

  - **host**: (string) host to connect,
  - **port**: (int) port number,
  - **family**: (string) internet family: ipv6 or ipv4 (Optional, default: ipv4).

The options for **unix** type:

  - **path**: (string) Required. The file path to the socket.

**Example for internet transports**

````ini
[connect]
type = "internet"
host = "localhost"
port = "9999"
family = "ipv6"
````

**Example for unix transports**

````ini
[connect]
type = "unix"
path = "/tmp/irccd.sock"
````

# The alias section

The alias section can be used to define custom user commands.

To define an alias, just add a new section named `[alias.name]` where name is
your desired alias name.

Then, add any option you like to execute commands you want. The option name is
ignored and serves as auto-documentation only.

Example:

````ini
[alias.present]
say-hello = ( "server-message", "localhost", "#staff", "hello world!" )
warning = ( "server-me", "localhost", "#staff", "is a bot")
````

This example defines an alias `present` that will:

  1. Send a message on the channel #staff in the server localhost
  2. Send an action emote on the same channel

To use this alias, call `irccdctl present`.

## Placeholders

Sometimes, you want to pass parameters to your alias. The placeholder syntax
allows you to define where your command line arguments will be replaced before
being sent to irccd.

The syntax uses `%n` where **n** is an integer starting from 0.

As you have seen in the `present` alias example above, the channel and server
are hardcoded so the user is not able to use this alias for different channels.
Let's update this alias with placeholders to make it more generic.

Example:

````ini
[alias.present]
say-hello = ( "server-message", "%0", "%1", "hello world!" )
warning = ( "server-me", "%0", "%1", "is a bot")
````

Now, the `present` alias will except two arguments from the command line when
the user invokes `irccdctl present`. Thus if you want to use this alias on the
**#staff@localhost**, you call the alias using
`irccdctl present localhost #staff`
