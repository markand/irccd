---
function: format
summary: >
  Format a string according to the [Common patterns and formatting](../../../../guide.html#common-patterns-and-formatting)
  specification.
synopsis: "str = Irccd.Util.format(input, params)"
arguments:
  - "input, the text to update"
  - "params, the parameters"
---


## Example

Replaces the keyword `message` and formats it bold and red.

````javascript
function onMessage(server, channel, origin, message)
{
    var s = Irccd.Util.format("@{red,default,bold}#{message}@{}", { message: message })
}
````

## Security

Be very careful when you use this function with untrusted input.

<div class="alert alert-danger" role="alert">
**Bad code**: Do never pass untrusted content (e.g. user message) as input parameter. For example, the following code is terribly dangerous:
</div>

````javascript
function onMessage(server, channel, origin, message)
{
    server.message(channel, Irccd.Util.format("%{red}" + input %{}");
}
````

If a user write a message like `${HOME}`, it will prints the user home directory, which is a high security issue
if you have environment variables with passwords.

<div class="alert alert-success">
**Correct code**: Instead, always use a literal string using a replacement with the user input
</div>

````javascript
function onMessage(server, channel, origin, message)
{
    server.message(channel, Irccd.Util.format("%{red}#{message}%{}", { message: message });
}
````
