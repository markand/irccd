# Event onWhois

This event is triggered when irccd gets information about a user.

# Synopsis

```javascript
function onWhois(server, info)
```

# Arguments

  - **server**: the current server,
  - **info**: the whois information.

The info is an object with the following properties:

  - **nickname**: the user nickname,
  - **user**: the user name,
  - **host**: the hostname,
  - **realname**: the real name used,
  - **channels**: an optional list of channels joined.
