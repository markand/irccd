# History plugin

The plugin **history** is used to check when someone has been seen for the last
time on a channel. For that purpose, irccd needs to be on that channel forever
if possible.

## Installation

The plugin **history** is distributed with irccd. To enable it add the following
to your `plugins` section:

```ini
[plugins]
history = ""
```

## Usage

The plugin **history** only reacts to the special command. It understands `seen`
and `said` sub commands.

- The sub command `seen` tells you when the user has been seen for the last time.
- The sub command `said` tells you what the user has said for the last time.

Example:

```nohighlight
markand: !history seen jean
irccd: markand, the last time I've seen jean was on 18/01/1989 at 20:55
markand: !history said jean
irccd: markand, the last thing that jean said was: hello world
```

## Configuration

The following options are available under the `[plugin.history]` section:

- file: (string) path to the JSON file for saving information (Optional,
  default to cache directory).

### Keywords supported

The following keywords are supported:

| Parameter | Keywords        |
|-----------|-----------------|
| **file**  | channel, server |

## Formats

The **history** plugin supports the following formats in `[format.history]`
section:

- error: (string) format when an internal error occured,
- seen: (string) format for showing last seen,
- said: (string) format for showing the last message,
- unknown: (string) format when the user has never been seen,
- usage: (string) format to show the plugin usage.

### Keywords supported

The following keywords are supported:

| Format      | Keywords                                           | Notes                        |
|-------------|----------------------------------------------------|------------------------------|
| (any)       | channel, command, nickname, origin, plugin, server |                              |
| **seen**    | target, (date)                                     | target is the specified nick |
| **said**    | message, target, (date)                            |                              |
| **unknown** | target                                             |                              |

Example:

```ini
[format.history]
seen = "#{target} has been seen on #{channel} the last time on: %d/%m/%Y %H:%M"
```
