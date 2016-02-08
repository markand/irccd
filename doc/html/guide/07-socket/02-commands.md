## Commands

The following commands are available. Please note that a lot of commands require a server as the first argument, it’s
one of defined in the **irccd.conf** file in the server section.

### server-cnotice

Send a message notice on a channel.

#### Properties

  - **command**: (string) "server-cnotice",
  - **server**: (string) the server unique id,
  - **channel**: (string) the channel name,
  - **message**: (string) the notice message.

#### Example

````json
{
  "command": "server-cnotice",
  "server": "myserver",
  "channel": "#staff",
  "message": "please be quiet"
}
````

### server-connect

Connect to a server.

#### Properties

  - **command**: (string) "server-connect",
  - **name**: (string) the server unique id,
  - **host**: (string) the host address,
  - **port**: (int) the port number (Optional, default: 6667),
  - **ssl**: (bool) use SSL (Optional, default: false),
  - **sslVerify**: (bool) verify SSL (Optional, default: false),
  - **nickname**: (string) the nickname to use (Optional, default: irccd),
  - **username**: (string) the user name to use (Optional, default: irccd),
  - **realname**: (string) the real name to use (Optional, default: IRC Client Daemon),
  - **ctcpVersion**: (string) the CTCP Version to answer (Optional, default: the irccd's version),
  - **commandChar**: (string) the command character to use to invoke commands (Optional, default: !),
  - **reconnectTries**: (int) the number of reconnection to try (Optional, default: -1),
  - **reconnectTimeout**: (int) the number of seconds to wait before retrying to connect (Optional, default: 30).

#### Example

````json
{
  "command": "server-connect",
  "name": "myserver",
  "host": "localhost",
  "nickname": "edouard"
}
````

### server-disconnect

Disconnect from a server.

If server is not specified, irccd disconnects all servers.

#### Properties

  - **command**: (string) "server-disconnect",
  - **server**: (string) the server unique id (Optional, default: none).

#### Example

````json
{
  "command": "server-disconnect",
  "server": "myserver"
}
````

### server-info

Get server information.

#### Properties

  - **command**: (string) "server-info",
  - **server**: (string) the server unique id.

#### Example

````json
{
  "command": "server-info",
  "server": "myserver"
}
````

#### Responses

  - **name**: (string) the server unique id,
  - **host**: (string) the server hostname,
  - **port**: (int) the port,
  - **ipv6**: (bool) true if using IPv6,
  - **ssl**: (bool) true if connection is using SSL,
  - **sslVerify**: (bool) true if SSL was verified,
  - **channels**: (string list) list of channels.
  - **nickname**: (string) the current nickname in use,
  - **username**: (string) the username in use,
  - **realname**: (string) the realname in use.

### server-invite

Invite the specified target on the channel.

#### Properties

  - **command**: (string) "server-invite",
  - **server**: (string) the server unique id,
  - **target**: (string) the nickname to invite,
  - **channel**: (string) the channel.

#### Example

````json
{
  "command": "server-invite",
  "server": "myserver",
  "target": "edouard",
  "channel": "#staff"
}
````

### server-join

Join the specified channel, the password is optional.

#### Properties

  - **command**: (string) "server-join",
  - **server**: (string) the server unique id,
  - **channel**: (string) the channel to join,
  - **password**: (string) the password (Optional, default: none).

#### Example

````json
{
  "command": "server-join",
  "server": "myserver",
  "channel": "#games"
}
````

### server-kick

Kick the specified target from the channel, the reason is optional.

#### Properties

  - **command**: (string) "server-kick",
  - **server**: (string) the server unique id,
  - **target**: (string) the target nickname,
  - **channel**: (string) the channel,
  - **reason**: (string) the reason (Optional, default: none).

#### Example

````json
{
  "command": "server-kick",
  "server": "myserver",
  "target": "edouard",
  "channel": "#staff",
  "reason": "please be nice"
}
````

### server-list

Get the list of all connected servers.

#### Properties

- **command**: (string) "server-list".

#### Example

````json
{
  "command": "server-list"
}
````

#### Responses

  - The following properties:
    - **list**: (string list) the list of all server unique ids.

### server-me

Send an action emote.

#### Properties

  - **command**: (string) "server-me",
  - **server**: (string) the server unique id,
  - **target**: (string) the target or channel,
  - **message**: (string) the message.

#### Example

````json
{
  "command": "server-me",
  "server": "myserver",
  "channel": "#staff",
  "message": "like that"
}
````

### server-message

Send a message to the specified target or channel.

#### Properties

  - **command**: (string) "server-message",
  - **server**: (string) the server unique id,
  - **target**: (string) the target or channel,
  - **message**: (string) the message.

#### Example

````json
{
  "command": "server-message",
  "server": "myserver",
  "target": "#staff",
  "message": "this channel is nice"
}
````

### server-mode

Set the irccd's user mode.

#### Properties

  - **command**: (string) "server-mode",
  - **server**: (string) the server unique id,
  - **mode**: (string) the mode.

#### Example

````json
{
  "command": "server-mode",
  "server": "myserver",
  "mode": "mode"
}
````

### server-nick

Change irccd's nickname.

#### Properties

  - **command**: (string) "server-nick",
  - **server**: (string) the server unique id,
  - **nickname**: (string) the new nickname.

#### Example

````json
{
  "command": "server-nick",
  "server": "myserver",
  "nickname": "edouard"
}
````

### server-notice

Send a private notice to the specified target.

#### Properties

  - **command**: (string) "server-notice",
  - **server**: (string) the server unique id,
  - **target**: (string) the target,
  - **message**: (string) the notice message.

#### Example

````json
{
  "command": "server-notice",
  "server": "myserver",
  "target": "edouard",
  "message": "hello dude"
}
````

### server-part

Leave the specified channel, the reason is optional.

Not all IRC servers support giving a reason to leave a channel, do not specify it if this is a concern.

#### Properties

  - **command**: (string) "server-part",
  - **server**: (string) the unique server id,
  - **channel**: (string) the channel to leave,
  - **reason**: (string) the reason (Optional, default: none).

#### Example

````json
{
  "command": "server-part",
  "server": "myserver",
  "channel": "#staff",
  "reason": "the reason"
}
````

### server-reconnect

Force reconnection of one or all servers.

If server is not specified, all servers will try to reconnect.

#### Properties

  - **command**: (string) "server-reconnect",
  - **server**: (string) the server unique id (Optional, default: none).

#### Example

````json
{
  "command": "server-reconnect",
  "server": "myserver"
}
````

### server-topic

Change the topic of the specified channel.

#### Properties

  - **command**: (string) "server-topic",
  - **server**: (string) the unique server id,
  - **channel**: (string) the channel,
  - **topic**: (string) the new topic.

#### Example

````json
{
  "command": "server-topic",
  "server": "myserver",
  "channel": "#staff",
  "topic": "the new topic"
}
````

### server-umode

Change your irccd user mode for the specified server.

````json
{
  "command": "umode",
  "server": "the server name",
  "mode": "the mode"
}
````
