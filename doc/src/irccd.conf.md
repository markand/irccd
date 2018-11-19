Both `irccd` and `irccdctl` use configuration file in a extended [INI][ini]
format.

# General syntax

The file syntax has following rules:

1. Each option is stored in a section,
2. Some sections may be redefined multiple times,
3. Empty option must have quotes (e.g. `option = ""`).

# The @include and @tryinclude statements

Irccd adds an extension to this format by adding an `@include` keyword which
let you splitting your configuration file.

note: this `@include` statement must be at the beginning of the file and must be
      surrounded by quotes if the file name has spaces.

You can use both relative or absolute paths. If relative paths are used, they
are relative to the current file being parsed.

The alternative `@tryinclude` keyword is similar but does not fails if the
requested file is not found.

## Example

```ini
@include "rules.conf"
@include "servers.conf"

[mysection]
myoption = "1"
```

# The list construct

When requested, an option can have multiples values in a list. The syntax uses
parentheses and values are separated by commas.

If the list have only one value, you can just use a simple string.

## Examples

```ini
[rule]
servers = ( "server1", "server2" )
```

```ini
[rule]
servers = "only-one-server"
```

Note: spaces are completely optional.

# Identifiers

Some sections require an identifier (specified as id) as parameter. They must be
unique, not empty and can only contain characters, numbers, '-' and '_'.

Example:

- abc
- server-tz2

# The logs section

This section can let you configure how irccd should log the messages.

The available options:

- verbose: (bool) be verbose (Optional, default: false),
- type: (string) which kind of logging, console, file or syslog
  (Optional, default: console).

The options for **file** type:

- path-logs: (string) path to the normal messages,
- path-errors: (string) path to the error messages.

note: syslog is not available on all platforms.

## Example

```ini
[logs]
type = file
verbose = true
path-logs = "/var/log/irccd/log.txt"
path-errors = "/var/log/irccd/errors.txt"
```

# The format section

The format section let you change the irccd's output. It uses the templates
system.

Only one keyword is defined, `message` which contains the message that irccd
wants to output.

note: colors and attributes are not supported on Windows.

The available options:

- debug: (string) template to use to format debug messages
  (Optional, default: none),
- info: (string) template to use to format information messages
  (Optional, default: none),
- warning: (string) template to use to format warnings
  (Optional, default: none).

## Example

```ini
[format]
debug = "%H:%M debug: #{message}"
info = "%H:%M info: #{message}"
warning = "%H:%M warning: #{message}"
```

# The server section

This section is used to connect to one or more server.

The available options:

- name: (id) the unique id,
- hostname: (string) the server address,
- port: (int) the server port (Optional, default: 6667),
- password: (string) an optional password
  (Optional, default: none),
- join-invite: (bool) join channels upon invitation
  (Optional, default: false),
- channels: (list) list of channels to auto join
  (Optional, default: empty),
- command-char: (string) the prefix for invoking special commands
  (Optional, default: !),
- ssl: (bool) enable or disable SSL (Optional, default: false),
  (Optional, default: true),
- auto-reconnect: (bool) enable reconnection after failure
  (Optional, default: true),
- auto-reconnect-delay: (int) number of seconds to wait before retrying
  (Optional, default: 30),
- ping-timeout (int) number of seconds before ping timeout
  (Optional, default: 300).
- nickname: (string) the nickname
  (Optional, default: system username if available or irccd),
- realname: (string) the realname
  (Optional, default: IRC Client Daemon),
- username: (string) the username name
  (Optional, default: system username if available or irccd),
- ctcp-version: (string) what version to respond to CTCP VERSION
  (Optional, default: IRC Client Daemon).

note: if a channel requires a password, add it after a colon
      (e.g. "#channel:password").

## Example

```ini
[server]
name = "local"
hostname = "localhost"
port = 6667
channels = ( "#staff", "#club:secret" )
```

# The paths section

The paths section defines common paths used as defaults for all plugins.

Any option in this section can be defined altough the following are used as
common convention used in all plugins:

- cache: (string) path for data files written by the plugin,
- data: (string) path for data files provided by the user,
- config: (string) path for additional configuration from the user.

For each of these paths, **plugin/name** is appended with the appropriate
plugin name when loaded.

The section is redefinable per plugin basis using the `[paths.<plugin>]` syntax.

## Example

```ini
#
# Common for all plugins.
#
# Example with ask plugin:
#
#   cache  -> /var/cache/irccd/plugin/ask
#   config -> /usr/local/etc/irccd/plugin/ask
#   data   -> /var/data/irccd/plugin/ask
#
[paths]
cache = "/var/cache/irccd"
config = "/usr/local/etc/irccd"
data = "/var/data/irccd"

#
# Explicit override for plugin hangman.
#
[paths.hangman]
config = "/etc/hangman"
```

# The plugins section

This section is used to load plugins.

Just add any key you like to load a plugin. If the value is not specified, the
plugin is searched through the standard directories, otherwise, provide the full
path (including the .js extension).

warning: remember to add an empty string for searching plugins.

## Example

```ini
[plugins]
history = ""
myplugin = /tmp/myplugin.js
```

The `history` plugin will be searched while `myplugin` will be load from
**/tmp/myplugin.js**.

# The transport section

This section defines transports, you may use sockets to do a basic IPC system
within irccd.

With transports, you can may ask `irccd` to send a message, a notice or even
kicking someone from a channel. Irccd will also notify all clients connected to
this transport on IRC events.

There are two type of listeners availables:

1. Internet sockets, IPv4 and IPv6,
2. Unix sockets, based on files (not available on Windows).

If SSL support was built in, both internet and unix sockets can be set to use
encrypted connections.

The available options:

- type: (string) type of listener "ip" or "unix".
- password: (string) an authentication password (Optional, default: none).
- ssl: (bool) enable SSL (Optional, default: false),
- key: (string) path to private key file (Required if ssl is true)
- certificate: (string) path to certificate (Required if ssl is true)

The options for **ip** type:

- port: (int) port number,
- address: (string) address to bind or "\*" for any
  (Optional, default: \*),
- ipv4: (bool) bind on IPv4 (Optional, default true),
- ipv6: (bool) bind on IPv6 (Optional, default true),

The options for **unix** type:

- path: (string) the file path to the socket.

## Example of internet transports

```ini
[transport]
type = "ip"
address = "*"
port = 9999
```

This will let you controlling irccd on port 9999 with both IPv4 and IPv6
families.

warning: consider using internet sockets with care, especially if you are
         running your bot on a server with multiple users. If your bot has
         operator rights and you bind on any address, almost every users
         can do a kick or a ban.

## Example of unix transports

```ini
[transport]
type = "unix"
path = "/tmp/irccd.sock"
```

This will let you controlling irccd on path **/tmp/irccd.sock**, the file is
automatically deleted when irccd starts, but not when it stops.

# The rule section

The rule section is one of the most powerful within irccd configuration. It let
you enable or disable plugins and IRC events for specific criterias. For
instance, you may want to disable a plugin only for a specific channel on a
specific server. And because rules are evaluated in the order they are defined,
you can override rules.

The available options:

- servers, (list) a list of servers that will match the rule
  (Optional, default: empty),
- channels, (list) a list of channel (Optional, default: empty),
- origins, (list) a list of nicknames to check (Optional, default: empty),
- plugins, (list) which plugins (Optional, default: empty),
- events, (list) which events (e.g onCommand, onMessage, ...)
  (Optional, default: empty),
- action, (string) set to **accept** or **drop**.

warning: don't make sensitive rules on **origins** option, irccd does not have
         any kind of nickname authentication. Thus, it may be very easy for
         someone to use a temporary nickname.

## Basic rules example

This first rule disable the plugin reboot on **all** servers and channels.

```ini
[rule]
plugins = "reboot"
action = drop
```

This rule enable the reboot plugin again on the server **localhost**,
channel **#staff**.

```ini
[rule]
servers = "localhost"
channels = "#staff"
plugins = "reboot"
action = accept
```

# Full example of configuration file

```ini
# Add a transport that bind only to IPv6.
[transport]
type = ip
ipv4 = false
ipv6 = true
family = ipv6
port = 12000

# A transport that binds to both IPv4 and IPv6.
[transport]
type = ip
port = 15000

# A server.
[server]
name = "foo"
host = "irc.foo.org"
port = "6667"
nickname = "superbot"
realname = "SuperBot v1.0"
username = "sp"

# An other server.
[server]
name = "wanadoo"
host = "chat.wanadoo.fr"
port = "6667"

# Load some plugins.
[plugins]
ask = ""                               # Search ask
myplugin = /path/to/myplugin.js        # Use absolute path
```

[ini]: https://en.wikipedia.org/wiki/INI_file

