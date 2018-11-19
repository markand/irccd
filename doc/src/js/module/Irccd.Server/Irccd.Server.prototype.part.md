# Method Irccd.Server.prototype.part

Leave the specified channel, the reason is optional.

Note: not all IRC servers support giving a reason to leave a channel, do not
      specify it if this is a concern.

# Synopsis

```javascript
Server.prototype.part(channel, reason)
```

# Arguments

- channel: the channel to leave,
- reason: an optional reason.
