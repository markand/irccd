# Event onKick

This event is triggered when someone has been kicked from a channel.

# Synopsis

```javascript
function onKick(server, origin, channel, target, reason)
```

# Arguments

  - **server**: the current server,
  - **origin**: who kicked the person,
  - **channel**: the channel,
  - **target**: the kicked person,
  - **reason**: an optional reason.
