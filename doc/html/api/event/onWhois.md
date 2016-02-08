---
event: onWhois
summary: "This event is triggered when irccd get information from a user."
synopsis: "function onWhois(server, info)"
arguments:
  - "server, the current server,"
  - "info, the whois information."
---

The info is a table with the following fields:

- **nickname**, the user nickname.
- **user**, the user name.
- **host**, the hostname.
- **realname**, the real name used.
- **channels**, an optional sequences of channels joined.
