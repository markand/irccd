# Function Irccd.Util.format

Format a string according to the template system.

# Synopsis

```javascript
str = Irccd.Util.format(input, params)
```

# Arguments

  - **input**: the text to update,
  - **params**: the parameters.

# Returns

The converted text.

# Example

Replaces the keyword `message` and formats it bold and red.

```javascript
function onMessage(server, channel, origin, message)
{
    var s = Irccd.Util.format("@{red,default,bold}#{message}@{}", { message: message })
}
```

## Security

Be very careful when you use this function with untrusted input.

Danger: Do never pass untrusted content (e.g. user message) as input parameter.

For example, the following code is terribly dangerous:

```javascript
function onMessage(server, channel, origin, message)
{
    // DANGEROUS.
    server.message(channel, Irccd.Util.format("@{red}" + message + "@{}");
}
```

If a user sends a message like `${HOME}`, it will prints the user home
directory, which is a high security issue if you have environment variables with
passwords.

Instead, always use a literal string using a replacement with the user input:

```javascript
function onMessage(server, channel, origin, message)
{
    // CORRECT.
    server.message(channel, Irccd.Util.format("@{red}#{message}@{}", { message: message });
}
```
