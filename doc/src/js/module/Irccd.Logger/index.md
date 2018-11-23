# Module Irccd.Logger

This module must be used to log something. It will add messages to the logging
system configured in the irccd.conf file.

For instance, if user has chosen to log into syslog, this module will log at
syslog too.

Any plugin can log messages, the message will be prepended by the plugin name to
be easily identifiable.

# Functions

- [debug](Irccd.Logger.debug.html)
- [info](Irccd.Logger.info.html)
- [warning](Irccd.Logger.warning.html)

# Example

```javascript
function onLoad()
{
    Irccd.Logger.info("This is an example");
}
```