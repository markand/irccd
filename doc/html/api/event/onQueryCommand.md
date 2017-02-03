---
event: onQueryCommand
js: true
summary: "Execute special command in query."
synopsis: "function onQueryCommand(server, origin, message)"
arguments:
  - "**server**: the current server,"
  - "**origin**: who invoked the command,"
  - "**message**: the real message, without the ! part."
---

Synonym of [onCommand](@baseurl@api/event/onCommand.html) but for queries.
