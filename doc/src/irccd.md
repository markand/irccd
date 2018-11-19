The `irccd` program is an IRC bot which connects to one or more severs and
dispatches events to plugins.

# Synopsis

    $ irccd [options...]

# Options

The following options are available:

- -c, --config file: specify the configuration file,
- -v, --verbose: be verbose,
- --version: show the version.

# Paths

Irccd uses different types of paths depending on the context.

- Configuration
- Plugins

Paths prefixed by (W) means they are only used on Windows, others prefixed by
(U) means they are used on Unix systems

# Configuration

The following directories are searched in the specified order for configuration
files. For example, the files `irccd.conf` and `irccdctl.conf` will be searched
there.

- \(W) %APPDATA%/irccd/config
- \(U) ${XDG\_CONFIG\_HOME}/irccd
- \(U) ${HOME}/.config/irccd (if XDG\_CONFIG\_HOME is not set)
- CMAKE\_INSTALL\_SYSCONFDIR/irccd

Examples:

- /home/john/.config/irccd/irccd.conf
- /usr/local/etc/irccd.conf
- C:/Program Files/irccd/etc/irccd/irccd.conf
- C:/Users/john/AppData/irccd/config

# Plugins

These directories are searched in the following order to load plugins when they
are not specified by full paths.

- current working directory
- \(W) %APPDATA%/irccd/share/plugins
- \(U) ${XDG\_DATA\_HOME}/irccd/plugins
- \(U) ${HOME}/.local/share/irccd/plugins (if XDG\_DATA\_HOME is not set)
- CMAKE\_INSTALL\_LIBDIR/irccd (both native and Javascript)

Examples:

- /home/john/.local/share/irccd/plugins/ask.js
- /usr/local/lib/irccd/plugins/ask.js
- C:/Users/john/AppData/irccd/share/plugins/ask.js
- C:/Program Files/irccd/lib/irccd/plugins/ask.js
