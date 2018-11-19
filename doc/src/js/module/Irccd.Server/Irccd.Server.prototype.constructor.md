# Function Irccd.Server (constructor)

Construct a new server

# Synopsis

```javascript
Irccd.Server(properties)
```

# Arguments

- properties: object information.

The properties argument may have the following properties:

- name: the name,
- hostname: the host,
- ipv4: enable ipv4 (Optional: default true)
- ipv6: enable ipv6, (Optional: default true)
- port: the port number, (Optional: default 6667)
- password: the password, (Optional: default none)
- channels: array of channels (Optiona: default empty)
- ssl: true to use ssl, (Optional: default false)
- nickname: "nickname", (Optional, default: irccd)
- username: "user name", (Optional, default: irccd)
- realname: "real name", (Optional, default: IRC Client Daemon)
- commandChar: "!", (Optional, the command char, default: "!")

warning: at least ipv4 and ipv6 must be set (which is the default).

# Example

```javascript
var s = new Irccd.Server({
	name: "localhost",
	hostname: "localhost",
	nickname: "kevin",
	ssl: true,
});
```
