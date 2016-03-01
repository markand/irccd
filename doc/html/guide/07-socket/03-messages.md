## Messages

The following messages are broadcasted to the clients on specific events.

Events are very close to the [JavaScript events][events], refer to the documentation for more information.

### onChannelMode

#### Properties

  - **event**: (string) "onChannelMode",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel,
  - **mode**: (string) the mode,
  - **argument**: (string) the argument.

### onChannelNotice

#### Properties

  - **event**: (string) "onChannelNotice",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel,
  - **message**: (string) the notice.

### onConnect

#### Properties

  - **event**: (string) "onConnect",
  - **server**: (string) the server id.

### onInvite

#### Properties

  - **event**: (string) "onInvite"
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel.

### onJoin

#### Properties

  - **event**: (string) "onJoin",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel.

### onKick

#### Properties

  - **event**: (string) "onKick",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel,
  - **target**: (string) the target,
  - **reason**: (string) the reason.

### onMessage

#### Properties

  - **event**: (string) "onMessage",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel,
  - **message**: (string) the message.

### onMe

#### Properties

  - **event**: (string) "onMe",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **target**: (string) the target,
  - **message**: (string) the message.

### onMode

#### Properties

  - **event**: (string) "onMode",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **mode**: (string) the mode.

### onNames

#### Properties

  - **event**: (string) "onNames",
  - **server**: (string) the server id,
  - **channel**: (string) the channel,
  - **names**: (string list) the list of names.

### onNick

#### Properties

  - **event**: (string) "onNick",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **nickname**: (string) the new nickname.

### onNotice

#### Properties

  - **event**: (string) "onNotice",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **message**: (string) the message.

### onPart

#### Properties

  - **event**: (string) "onPart",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel,
  - **reason**: (string) the reason.

### onQuery

#### Properties

  - **event**: (string) "onQuery",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **message**: (string) the message.

### onTopic

#### Properties

  - **event**: (string) "onTopic",
  - **server**: (string) the server id,
  - **origin**: (string) the originator,
  - **channel**: (string) the channel,
  - **topic**: (string) the topic.

### onWhois

#### Properties

  - **event**: (string) "onWhois",
  - **server**: (string) the server id,
  - **nickname**: (string) the nickname,
  - **username**: (string) the username,
  - **host**: (string) the hostname,
  - **realname**: (string) the realname.

[events]: @baseurl@/api/index.html
