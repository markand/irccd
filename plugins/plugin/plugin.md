# Plugin plugin

The plugin **plugin** let you inspect loaded plugins.

## Installation

The plugin **plugin** is distributed with irccd. To enable it add the following
to your **plugins** section:

```ini
[plugins]
plugin = ""
```

## Usage

The plugin **plugin** only reacts to the special command. It understands `info`
and `list` sub commands.

- The sub command `info` shows information about a plugin,
- The sub command `list` shows loaded plugins (see configuration for limits).

Both commands work in a channel or as private message with irccd.

## Configuration

The following options are available under the `[plugin.plugin]` section:

- **max-list-lines**: (int) max number of lines allowed for the `list` sub
  command (Optional, default: 3),
- **max-list-columns**: (int) max number of columns allowed per lines
  (Optional, default: 80).

## Formats

The **plugin** plugin supports the following formats in `[format.plugin]`
section:

- usage: (string) message to show on invalid usage,
- info: (string) plugin information message to show,
- not-found: (string) message to show if a plugin does not exist,
- too-long: (string) message to show if the list of plugin is too long.

### Keywords supported

The following keywords are supported:

| Format        | Keywords                                           | Notes                  |
|---------------|----------------------------------------------------|------------------------|
| (any)         | channel, command, nickname, origin, plugin, server |                        |
| **info**      | author, license, name, summary, version            | the plugin information |
| **not-found** | name                                               | the plugin name        |
