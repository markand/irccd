## Irccdctl commands

The following commands are available.

### help

Get the help.

**Usage**

````
$ irccdctl help subject
````

**Example**

````
$ irccdctl help server-message
````

### plugin-info

Get plugin information.

**Usage**

````
$ irccdctl plugin-info name
````

**Example**

````
$ irccdctl plugin-info ask
````

### plugin-list

Get the list of all loaded plugins.

**Usage**

````
$ irccdctl plugin-list
````

### plugin-load

Load a plugin into the irccd instance.

**Usage**

````
$ irccdctl plugin-load plugin
````

**Example**

````
$ irccdctl load ask
````

### plugin-reload

Reload a plugin, parameter name is the plugin to reload.

The plugin must be loaded.

**Usage**

````
$ irccdctl plugin-reload name
````

**Example**

````
$ irccdctl plugin-reload logger
````

### plugin-unload

Unload a loaded plugin from the irccd instance.

**Usage**

````
$ irccdctl plugin-unload name
````

**Example**

````
$ irccdctl plugin-unload logger
````

### server-cmode

Change the mode of the specified channel.

**Usage**

````
$ irccdctl server-cmode server channel mode
````

**Example**

````
$ irccdctl server-cmode freenode #staff +t
````

### server-cnotice

Send a notice to a public channel. This is a notice that everyone will be notified by.

**Usage**

````
$ irccdctl server-cnotice server channel message
````

**Usage**

````
$ irccdctl server-cnotice freenode #staff "Don't flood"
````

### server-connect

Connect to a new IRC server.

**Usage**

````
$ irccdctl server-connect [options] name host port
````

Available options:

- **-c, --command**: specify the command char
- **-n, --nickname**: specify a nickname
- **-r, --realname**: specify a real name
- **-S, --ssl-verify**: verify SSL
- **-s, --ssl**: connect using SSL
- **-u, --username**: specify a user name

**Example**

````
$ irccdctl server-connect wanadoo chat.wanadoo.fr 6667
$ irccdctl server-connect -s -S -n "undead" wanadoo chat.wanadoo.fr 6697
````

### server-disconnect

Disconnect from a connected server.

**Usage**

````
$ irccdctl server-disconnect name
````

**Example**

````
$ irccdctl server-disconnect wanadoo
````

### server-invite

Invite someone to a channel, needed for channel with mode +i

**Usage**

````
$ irccdctl server-invite server nickname channel
````

**Example**

````
$ irccdctl server-invite freenode xorg62 #staff
````

### server-join

Join the specified channel, the password is optional.

**Usage**

````
$ irccdctl server-join server channel [password]
````

**Example**

````
$ irccdctl server-join freenode #staff
````

### server-kick

Kick the specified target from the channel, the reason is optional.

**Usage**

````
$ irccdctl server-kick server target channel [reason]
````

**Example**

````
$ irccdctl kick freenode jean #staff "Stop flooding"
````

### server-list

Get the list of all connected servers.

**Usage**

````
$ irccdctl server-list
````

### server-me

Send an action emote.

**Usage**

````
$ irccdctl server-me server target message
````

**Example**

````
$ irccdctl server-me freenode #staff "going back soon"
````

### server-message

Send a message to the specified target or channel.

**Usage**

````
$ irccdctl server-message server target message
````

**Example**

````
$ irccdctl server-message freenode #staff "Hello from irccd"
````

### server-mode

Set the irccd's user mode.

**Usage**

````
$ server-mode server mode
````

**Example**

````
$ irccdctl server-mode +i
````

### server-nick

Change irccd's nickname.

**Usage**

````
$ irccdctl server-nick server nickname
````

**Example**

````
$ irccdctl server-nick freenode david
````

### server-notice

Send a private notice to the specified target.

**Usage**

````
$ irccdctl server-notice server target message
````

**Example**

````
$ irccdctl server-notice freenode jean "I know you are here."
````

### server-part

Leave the specified channel, the reason is optional.

<div class="alert alert-warning" role="alert">
**Warning**: not all IRC servers support giving a reason to leave a channel, do not specify it if this is a concern.
</div>

**Usage**

````
$ irccdctl server-part server channel [reason]
````

**Example**

````
$ irccdctl server-part freenode #staff
$ irccdctl server-part freenode #botwar "too noisy"
````

### server-reconnect

Force reconnection of one or all servers.

If server is not specified, all servers will try to reconnect.

**Usage**

````
$ irccdctl server-reconnect [server]
````

**Example**

````
$ irccdctl server-reconnect
$ irccdctl server-reconnect wanadoo
````

### server-topic

Change the topic of the specified channel.

**Usage**

````
$ irccdctl server-topic server channel topic
````

**Example**

````
$ irccdctl server-topic freenode #wmfs "This is the best channel"
````

### watch

Start watching irccd events. You can use different output formats, native is human readable format, json is pretty
formatted json.

**Usage**

````
$ irccdctl watch [-f|--format native|json]
````

**Example**

````
$ irccdctl watch
$ irccdctl watch -f json
````
