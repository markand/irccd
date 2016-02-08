---
event: onCommand
summary: "Execute special command."
synopsis: "function onCommand(server, origin, channel, message)"
arguments:
  - "server, the current server,"
  - "origin, who invoked the command,"
  - "channel, the channel where the message comes from,"
  - "message, the real message, without the ! part."
---

Special commands are not real IRC events. They are called from channel messages with a specific syntax using a delimiter
and the plugin name.

For instance, with default irccd parameters, saying on a channel `!ask foo` will call the special command of the plugin
named **ask**.
