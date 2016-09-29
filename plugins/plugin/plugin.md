---
title: "Plugin plugin"
header: "Plugin plugin"
guide: yes
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

The following options are available under the `[plugin.plugin]` section:

**Deprecated in irccd 2.1.0:**

  - **format-usage**: Use `[format.plugin] usage` instead,
  - **format-info**: Use `[format.plugin] info` instead,
  - **format-not-found**: Use `[format.plugin] not-found` instead,
  - **format-too-long**: Use `[format.plugin] too-long` instead,

## Formats

The **plugin** plugin supports the following formats in `[format.plugin]` section:

  - **usage**: (string) message to show on invalid usage,
  - **info**: (string) plugin information message to show,
  - **not-found**: (string) message to show if a plugin does not exist,
  - **too-long**: (string) message to show if the list of plugin is too long.

### Keywords supported

The following keywords are supported:

| Format        | Keywords                                           | Notes                                       |
|---------------|----------------------------------------------------|---------------------------------------------|
| (any)         | channel, command, nickname, origin, plugin, server |                                             |
| **info**      | author, license, name, summary, version            | the plugin information                      |
| **not-found** | name                                               | the plugin name                             |
