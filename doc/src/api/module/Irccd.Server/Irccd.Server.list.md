# Function Irccd.Server.list

List all servers in a map.

# Synopsis

```javascript
table = Irccd.Server.list()
```

# Returns

The table of all servers.

# Example

```javascript
var table = Irccd.Server.list();

for (var name in table) {
    var server = table[name];

    // Use server.
}
```
