---
title: "Plugin plugin"
header: "Plugin plugin"
---

The plugin **plugin** let you inspect loaded plugins.

## Installation

The plugin **plugin** is distributed with irccd. To enable it add the following to your **plugins** section:

````ini
[plugins]
plugin = ""
````

## Usage

The plugin **plugin** only reacts to the special command. It understands `info` and `list` sub commands.

  - The sub command `info` shows information about a plugin,
  - The sub command `list` shows loaded plugins.

Both commands work in a channel or as private message with irccd.

## Configuration

You can use different formats.

The following options are available under the `[plugin.plugin]` section:

  - **format-usage**: (string) message to show on invalid usage,
  - **format-info**: (string) plugin information message to show,
  - **format-not-found**: (string) message to show if a plugin does not exist,
  - **format-too-long**: (string) message to show if the list of plugin is too long.

### Keywords supported

The following keywords are supported:

| Format                  | Keywords                                           | Notes                                       |
|-------------------------|----------------------------------------------------|---------------------------------------------|
| (any)                   | channel, command, nickname, origin, plugin, server |                                             |
| **format-info**         | author, license, name, summary, version            | the plugin information                      |
| **format-not-found**    | name                                               | the plugin name                             |
