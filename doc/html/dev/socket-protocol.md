---
header: Irccd socket API
guide: true
---

This guide will help you controlling irccd via sockets.

Currently, irccd supports internet and unix sockets, you need at least one
transport defined in your **irccd.conf**.

# Syntax

Irccd use JSON as protocol for sending and receiving data. A message must ends
with `\r\n\r\n` to be complete, thus it's possible to write JSON messages in
multiple lines.

For example, this buffer will be parsed as two different messages.

<div class="alert alert-success" role="alert">
**Example**: two commands issued

````json
{
  "param1": "value1"
}

{
  "param1": "value1"
}

````
</div>

<div class="alert alert-warning" role="alert">
**Warning:** please note that the `\r\n\r\n`characters are the escape characters of line feed and new line, not the
concatenation of `\` and `r`.
</div>

## Responses

All commands emit a response with the following properties:

  - **command**: (string) the result of the issued command,
  - **status**: (string) **error** or **ok**,
  - **error**: (string) the error message if status is set to **error**.

<div class="alert alert-success" role="alert">
**Example**: command issued with no errors

````json
{
  "command": "server-message",
  "status": "ok"
}
````
</div>

<div class="alert alert-danger" role="alert">
**Example**: command issued with errors

````json
{
  "command": "server-message",
  "status": "error",
  "error": "server xyz not found"
}
````
</div>
