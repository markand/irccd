---
method: info
js: true
summary: "Get server information."
synopsis: "info = Server.prototype.info()"
returns: "The server information."
---

The returned object  has the following fields:

  - **name**: (string) the server unique name,
  - **host**: (string) the host name,
  - **port**: (int) the port number,
  - **ssl**: (bool) true if using ssl,
  - **sslVerify**: (bool) true if ssl was verified,
  - **channels**: (string list) an array of all channels,
  - **realname**: (string) the current real name,
  - **username**: (string) the user name,
  - **nickname**: (string) the current nickname.