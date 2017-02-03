---
header: Configuring irccd
guide: yes
---

To configure irccd, create a `irccd.conf` file in one of the
[configuration directories][cfgdir].

Options that have a default value are optional and can be omitted.

# Identifiers

Some sections require an identifier (specified as id) as parameter. They must be unique, not empty and can only
contain characters, numbers, '-' and '_'.

Example:

  - abc
  - server-tz2

<div class="alert alert-info" role="alert">
**Note**: the regular expression is defined as `[A-Za-z0-9-_]+`.
</div>

# The general section

This section contains global options that are used in the whole irccd application.

The available options:

  - **uid**: (string or number) the user id to use (Optional, default: none),
  - **gid**: (string or number) the group id to use (Optional, default: none),
  - **foreground**: (bool) set to true to not daemonize (Optional, default: false)
  - **pidfile**: (string) path to a file where to store the irccd pid (Optional, default: none).

<div class="alert alert-warning" role="alert">
**Warning:** these options are available only if the system supports them.
</div>

**Example**

````ini
[general]
pidfile = "/var/run/irccd/pid"
uid = "nobody"
gid = 1002
````

# The logs section

This section can let you configure how irccd should log the messages.

The available options:

  - **verbose**: (bool) be verbose (Optional, default: false),
  - **type**: (string) which kind of logging, console, file or syslog (Optional, default: console).

The options for **file** type:

  - **path-logs**: (string) path to the normal messages,
  - **path-errors**: (string) path to the error messages.

<div class="alert alert-info" role="alert">
**Note:** syslog is not available on all platforms.
</div>

**Example**

````ini
[logs]
type = file
verbose = true
path-logs = "/var/log/irccd/log.txt"
path-errors = "/var/log/irccd/errors.txt"
````

# The format section

The format section let you change the irccd's output. It uses the [common patterns][cp].

Only one keyword is defined, `message` which contains the message that irccd wants to output.

<div class="alert alert-info" role="alert">
**Note:** the colors and attributes are not supported.
</div>

The available options:

  - **debug**: (string) template to use to format debug messages (Optional, default: none),
  - **info**: (string) template to use to format information messages (Optional, default: none),
  - **warning**: (string) template to use to format warnings (Optional, default: none).

**Example**

````ini
[format]
debug = "%H:%M debug: #{message}"
info = "%H:%M info: #{message}"
warning = "%H:%M warning: #{message}"
````

# The identity section

This section is completely optional, if you don't provide one, irccd will use a default identity with **irccd** as
nickname and username. This section is redefinable, you can create one or more.

<div class="alert alert-warning" role="alert">
**Warning:** it is encouraged to set a different identity because irccd nickname may be already used in some servers.
</div>

The available options:

  - **name**: (id) the identity unique id,
  - **nickname**: (string) the nickname (Optional, default: irccd),
  - **realname**: (string) the realname (Optional, default: IRC Client Daemon),
  - **username**: (string) the username name (Optional, default: irccd),
  - **ctcp-version**: (string) what version to respond to CTCP VERSION (Optional, default: IRC Client Daemon),
  - **ctcp-autoreply**: (bool) enable auto CTCP VERSION reply, (Optional, default: true).

**Example**

````ini
[identity]
name = "default"
nickname = "jean"

[identity]
name = "development"
nickname = "unstable"
username = "un"
````

# The server section

This section is used to connect to one or more server. Thus, this section is also redefinable.

The available options:

  - **name**: (id) the unique id,
  - **host**: (string) the server address,
  - **port**: (int) the server port (Optional, default: 6667),
  - **identity**: (string) an identity to use (Optional, default: irccd's default),
  - **password**: (string) an optional password (Optional, default: none),
  - **join-invite**: (bool) join channels upon invitation (Optional, default: false),
  - **channels**: (list) list of channels to auto join, (Optional, default: empty),
  - **command-char**: (string) the prefix for invoking special commands (Optional, default: !),
  - **ssl**: (bool) enable or disable SSL (Optional, default: false),
  - **ssl-verify**: (bool) verify the SSL certificates (Optional, default: true),
  - **reconnect**: (bool) enable reconnection after failure (Optional, default: true),
  - **reconnect-tries**: (int) number of tries before giving up. A value of -1 means indefinitely (Optional, default: -1),
  - **reconnect-timeout**: (int) number of seconds to wait before retrying (Optional, default: 30),
  - **ping-timeout** (int) number of seconds before ping timeout (Optional, default: 300).

<div class="alert alert-info" role="alert">
**Note:** if a channel requires a password, add it after a colon (e.g. "#channel:password").
</div>

**Example**

````ini
[server]
name = "local"
host = "localhost"
port = 6667
channels = ( "#staff", "#club:secret" )
````

# The plugins section

This section is used to load plugins.

Just add any key you like to load a plugin. If the value is not specified, the plugin is searched through the standard
directories, otherwise, provide the full path (including the .js extension).

<div class="alert alert-warning" role="alert">
**Warning:** remember to add an empty string for searching plugins.
</div>

**Example**

````ini
[plugins]
history = ""
myplugin = /tmp/myplugin.js
````

The `history` plugin will be searched while `myplugin` will be load from **/tmp/myplugin.js**.

# The transport section

This section defines transports, you may use sockets to do a basic IPC system within irccd.

With transports, you can may ask `irccd` to send a message, a notice or even kicking someone from a channel. Irccd
will also notify all clients connected to this transport on IRC events.

See [irccdctl chapter][irccdctl] and the [socket chapter][sockets] for more information.

There are two type of listeners availables:

  1. Internet sockets, IPv4 and IPv6
  2. Unix sockets, based on files (not available on Windows)

The available options:

  - **type**: (string) type of listener "ip" or "unix".
  - **password**: (string) an authentication password (Optional, default: none).

The options for **ip** type:

  - **port**: (int) port number,
  - **address**: (string) address to bind or "\*" for any (Optional, default: \*),
  - **family**: (list) ipv6, ipv4. Both are accepted (Optional, default: ipv4),
  - **ssl**: (bool) enable SSL (Optional, default: false),
  - **key**: (string) path to private key file (Optional, default: none),
  - **certificate**: (string) path to certificate (Optional, default: none).

The options for **unix** type:

  - **path**: (string) the file path to the socket.

**Example of internet transports**

````ini
[transport]
type = "ip"
address = "*"
family = ( "ipv4", "ipv6" )
port = 9999
````

This will let you controlling irccd on port 9999 with both IPv4 and IPv6 families.

<div class="alert alert-warning" role="alert">
**Warning**: consider using internet sockets with care, especially if you are running your bot on a server with
multiple users. If your bot has operator rights and you bind on any address, almost every users can do a kick or a ban.
</div>

**Example of unix transports**

````ini
[transport]
type = "unix"
path = "/tmp/irccd.sock"
````

This will let you controlling irccd on path **/tmp/irccd.sock**, the file is automatically deleted when irccd starts,
but not when it stops.

# The rule section

The rule section is one of the most powerful within irccd configuration. It let you enable or disable plugins and IRC
events for specific criterias. For instance, you may want to disable a plugin only for a specific channel on a specific
server. And because rules are evaluated in the order they are defined, you can override rules.

The available options:

  - **servers**, (list) a list of servers that will match the rule (Optional, default: empty),
  - **channels**, (list) a list of channel (Optional, default: empty),
  - **plugins**, (list) which plugins (Optional, default: empty),
  - **events**, (list) which events (e.g onCommand, onMessage, ...) (Optional, default: empty),
  - **action**, (string) set to **accept** or **drop**.

## Basic rules example

This first rule disable the plugin reboot on **all** servers and channels.

````ini
[rule]
plugins = "reboot"
action = drop
````

This rule enable the reboot plugin again on the server **localhost**, channel **#staff**.

````ini
[rule]
servers = "localhost"
channels = "#staff"
plugins = "reboot"
action = accept
````

# Full example of configuration file

````ini
# Add a transport that bind only to IPv6.
[transport]
type = ip
family = ipv6
port = 12000

# A transport that binds to both IPv4 and IPv6.
[transport]
type = ip
family = ( ipv4, ipv6 )
port = 15000

# Identity reused by many servers.
[identity]
name = "myIdentity"
nickname = "superbot"
realname = "SuperBot v1.0"
username = "sp"

# A server.
[server]
name = "foo"
host = "irc.foo.org"
port = "6667"
identity = myIdentity

# An other server.
[server]
name = "wanadoo"
host = "chat.wanadoo.fr"
port = "6667"
identity = myIdentity

# Load some plugins.
[plugins]
ask = ""                               # Search ask
myplugin = /path/to/myplugin.js        # Use absolute path
````

[cfgdir]: @baseurl@irccd/paths.html
[cp]: @baseurl@misc/common-patterns-and-formatting.html
[irccdctl]: @baseurl@irccdctl/index.html
[sockets]: @baseurl@dev/socket-commands.html
