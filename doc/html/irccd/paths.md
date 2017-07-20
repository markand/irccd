---
header: Paths
guide: yes
---

Irccd uses different types of paths depending on the context.

  - Configuration
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
