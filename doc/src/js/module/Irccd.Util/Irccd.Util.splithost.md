# Function Irccd.Util.splithost

Extract the host from a user, for instance with **foo!~foo@localhost**,
**localhost** will be returned.

# Synopsis

```javascript
hostname = Irccd.Util.splithost(user)
```

# Arguments

- user: the user to split.

# Returns

The hostname.
