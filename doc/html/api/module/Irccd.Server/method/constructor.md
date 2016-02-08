---
method: constructor
summary: "Construct a new server"
synopsis: "Irccd.Server(params) /* constructor */"
arguments:
  - "params, parameters."
---

The params argument may have the following properties:

- **name**: the name,
- **host**: the host,
- **ipv6**: true to use ipv6, (Optional: default false)
- **port**: the port number, (Optional: default 6667)
- **password**: the password, (Optional: default none)
- **channels**: array of channels (Optiona: default empty)
- **ssl**: true to use ssl, (Optional: default false)
- **sslVerify**: true to verify (Optional: default true)
- **nickname**: "nickname", (Optional, default: irccd)
- **username**: "user name", (Optional, default: irccd)
- **realname**: "real name", (Optional, default: IRC Client Daemon)
- **commandChar**: "!", (Optional, the command char, default: "!")

Example

````javascript
var s = new Irccd.Server({
	name: "localhost",
	host: "localhost",
	nickname: "kevin",
	ssl: true,
	sslVerify: false
});
````
