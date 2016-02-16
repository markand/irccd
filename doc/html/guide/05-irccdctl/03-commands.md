## Irccdctl commands

The following commands are available.

### help

Get the help.

**Usage**

````nohighlight
$ irccdctl help subject
````

**Example**

````nohighlight
$ irccdctl help server-message
````

### plugin-info

Get plugin information.

**Usage**

````nohighlight
$ irccdctl plugin-info name
````

**Example**

````nohighlight
$ irccdctl plugin-info ask
````

### plugin-list

Get the list of all loaded plugins.

**Usage**

````nohighlight
$ irccdctl plugin-list
````

### plugin-load

Load a plugin into the irccd instance.

**Usage**

````nohighlight
$ irccdctl plugin-load plugin
````

**Example**

````nohighlight
$ irccdctl load ask
````

### plugin-reload

Reload a plugin by calling the appropriate onReload event, the plugin is not unloaded and must be already loaded.

**Usage**

````nohighlight
$ irccdctl plugin-reload name
````

**Example**

````nohighlight
$ irccdctl plugin-reload logger
````

### plugin-unload

Unload a loaded plugin from the irccd instance.

**Usage**

````nohighlight
$ irccdctl plugin-unload name
````

**Example**

````nohighlight
$ irccdctl plugin-unload logger
````

### server-cmode

Change the mode of the specified channel.

**Usage**

````nohighlight
$ irccdctl server-cmode server channel mode
````

**Example**

````nohighlight
$ irccdctl server-cmode freenode #staff +t
````

### server-cnotice

Send a notice to a public channel. This is a notice that everyone will be notified by.

**Usage**

````nohighlight
$ irccdctl server-cnotice server channel message
````

**Usage**

````nohighlight
$ irccdctl server-cnotice freenode #staff "Don't flood"
````

### server-connect

Connect to a new IRC server.

**Usage**

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

**Example**

````nohighlight
$ irccdctl server-connect wanadoo chat.wanadoo.fr 6667
$ irccdctl server-connect -s -S -n "undead" wanadoo chat.wanadoo.fr 6697
````

### server-disconnect

Disconnect from a connected server.

**Usage**

````nohighlight
$ irccdctl server-disconnect name
````

**Example**

````nohighlight
$ irccdctl server-disconnect wanadoo
````

### server-invite

Invite someone to a channel, needed for channel with mode +i

**Usage**

````nohighlight
$ irccdctl server-invite server nickname channel
````

**Example**

````nohighlight
$ irccdctl server-invite freenode xorg62 #staff
````

### server-join

Join the specified channel, the password is optional.

**Usage**

````nohighlight
$ irccdctl server-join server channel [password]
````

**Example**

````nohighlight
$ irccdctl server-join freenode #staff
````

### server-kick

Kick the specified target from the channel, the reason is optional.

**Usage**

````nohighlight
$ irccdctl server-kick server target channel [reason]
````

**Example**

````nohighlight
$ irccdctl kick freenode jean #staff "Stop flooding"
````

### server-list

Get the list of all connected servers.

**Usage**

````nohighlight
$ irccdctl server-list
````

### server-me

Send an action emote.

**Usage**

````nohighlight
$ irccdctl server-me server target message
````

**Example**

````nohighlight
$ irccdctl server-me freenode #staff "going back soon"
````

### server-message

Send a message to the specified target or channel.

**Usage**

````nohighlight
$ irccdctl server-message server target message
````

**Example**

````nohighlight
$ irccdctl server-message freenode #staff "Hello from irccd"
````

### server-mode

Set the irccd's user mode.

**Usage**

````nohighlight
$ server-mode server mode
````

**Example**

````nohighlight
$ irccdctl server-mode +i
````

### server-nick

Change irccd's nickname.

**Usage**

````nohighlight
$ irccdctl server-nick server nickname
````

**Example**

````nohighlight
$ irccdctl server-nick freenode david
````

### server-notice

Send a private notice to the specified target.

**Usage**

````nohighlight
$ irccdctl server-notice server target message
````

**Example**

````nohighlight
$ irccdctl server-notice freenode jean "I know you are here."
````

### server-part

Leave the specified channel, the reason is optional.

<div class="alert alert-warning" role="alert">
**Warning**: not all IRC servers support giving a reason to leave a channel, do not specify it if this is a concern.
</div>

**Usage**

````nohighlight
$ irccdctl server-part server channel [reason]
````

**Example**

````nohighlight
$ irccdctl server-part freenode #staff
$ irccdctl server-part freenode #botwar "too noisy"
````

### server-reconnect

Force reconnection of one or all servers.

If server is not specified, all servers will try to reconnect.

**Usage**

````nohighlight
$ irccdctl server-reconnect [server]
````

**Example**

````nohighlight
$ irccdctl server-reconnect
$ irccdctl server-reconnect wanadoo
````

### server-topic

Change the topic of the specified channel.

**Usage**

````nohighlight
$ irccdctl server-topic server channel topic
````

**Example**

````nohighlight
$ irccdctl server-topic freenode #wmfs "This is the best channel"
````

### watch

Start watching irccd events. You can use different output formats, native is human readable format, json is pretty
formatted json.

**Usage**

````nohighlight
$ irccdctl watch [-f|--format native|json]
````

**Example**

````nohighlight
$ irccdctl watch
$ irccdctl watch -f json
````
