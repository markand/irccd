---
function: list
summary: "List all servers in a map."
synopsis: "table = Irccd.Server.list()"
returns: "the table of all servers."
---

## Example

````javascript
var table = Irccd.Server.list();

for (var name in table) {
    var server = table[name];

    /* Use server */
}
````
