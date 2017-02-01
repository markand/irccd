---
header: Paths
guide: yes
---

Irccd uses different types of paths depending on the context.

  - Configuration
  - Data
  - Plugins

Paths prefixed by (W) means they are only used on Windows, others prefixed by
(U) means they are used on Unix.

# Configuration

The following directories are searched in the specified order for configuration
files. For example, the files `irccd.conf` and `irccdctl.conf` will be searched
there.

  - \(W) `%APPDATA%/irccd/config`
  - \(U) `${XDG_CONFIG_HOME}/irccd`
  - \(U) `${HOME}/.config/irccd` (if `XDG_CONFIG_HOME` is not set)
  - `installation-directory/etc`

Examples:

  - `/home/john/.config/irccd/irccd.conf`
  - `/usr/local/etc/irccd.conf`
  - `C:/Program Files/irccd/etc/irccd.conf`
  - `C:/Users/john/AppData/irccd/config`

# Data

The data directory is only used by plugins, it is dedicated to store important
files such as plugin assets, logs or anything that is meaningful for the user
or the plugin.

<div class="alert alert-info" role="alert">
**Note**: The plugins never try to create the directories, instead irccd
searches for the first available one and use it. If the directory does not exist
it is set by default to the **system** one.

It is thus recommended to create a directory into your local home folder if you
run irccd as your user and not as a system daemon.
</div>

The following directories as searched in order:

  - \(W) `%APPDATA%/irccd/share`
  - \(U) `${XDG_DATA_HOME}/irccd`
  - \(U) `${HOME}/.local/share/irccd` (if `XDG_DATA_HOME` is not set)
  - \(W) `installation-directory/share`
  - \(U) `installation-directory/share/irccd`

For plugins, the path is appended with `plugin/<plugin_name>` (e.g. plugin/ask).

Examples:

  - `/home/john/.local/share/irccd/plugin/ask`
  - `/usr/local/share/irccd/plugin/ask`
  - `C:/Users/john/AppData/irccd/share/plugin/ask`
  - `C:/Program Files/irccd/share/plugin/ask`

# Plugins

These directories are searched in the following order to load plugins when they
are not specified by full paths.

  - current working directory
  - \(W) `%APPDATA%/irccd/share/plugins`
  - \(U) `${XDG_DATA_HOME}/irccd/plugins`
  - \(U) `${HOME}/.local/share/irccd/plugins` (if `XDG_DATA_HOME` is not set)
  - \(W) `installation-directory/share/plugins`
  - \(U) `installation-directory/share/irccd/plugins`

Examples:

  - `/home/john/.local/share/irccd/plugins/ask.js`
  - `/usr/local/share/irccd/plugins/ask.js`
  - `C:/Users/john/AppDAta/irccd/share/plugins/ask.js`
  - `C:/Program Files/irccd/share/plugins/ask.js`
