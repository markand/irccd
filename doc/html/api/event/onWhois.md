---
event: onWhois
summary: "This event is triggered when irccd gets information about a user."
synopsis: "function onWhois(server, info)"
arguments:
  - "server, the current server,"
  - "info, the whois information."
---

The info is an object with the following properties:

  - **nickname**, the user nickname,
  - **user**, the user name,
  - **host**, the hostname,
  - **realname**, the real name used,
  - **channels**, an optional list of channels joined.
