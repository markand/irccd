# Event onMode

This event is triggered when the server changed a channel mode or your mode.

# Synopsis

```javascript
function onMode(server, origin, channel, mode, limit, user, mask)
```

# Arguments

  - **server**: the current server,
  - **origin**: the person who changed the mode,
  - **mode**: the new mode.
