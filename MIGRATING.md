IRC Client Daemon MIGRATING
===========================

This document is a small guide to help you migrating to a next major version.

Migrating from 4.x to 5.x
=========================

C API
-----

Most of the functions use a different naming scheme depending of the nature of
the type and how it is allocated.

When a type is designed to be dynamically allocated (such as owned types) they
are usually allocated on the heap using functions named `irc_<foo>_new` and
destroyed `irc_<foo>_free`. On the other hand, non-opaque and cheap types are
allocated on the user stack named `irc_<foo>_init` (or similar) and
`irc_<foo>_finish`.

Examples:

- `irc_server_new` / `irc_server_free`
- `irc_hook_new` / `irc_hook_free`

### irccd/channel.h

- A `irc_channel` has now an enum as flags (`enum irc_channel_flags`) rather
  than a unique `joined` field for future use.
- A new function `irc_channel_set` should be used to update a nickname
  information rather than modifying it directly.

### irccd/irccd.h

- Deferred functions such as `irc_bot_post` have been removed in favor of the
  libev library. Use `ev_timer` or `ev_async` if asynchronous operations are
  required.
- The `irc_pollable` interface has been removed for the same reason.
- The `irc_bot_plugin_find` has been renamed to `irc_bot_plugin_search` for a
  cleaner meaning.

### irccd/limits.h

The file has been removed as most of the strings are now dynamically allocated
for a cleaner interface and no more trillions of arbitrary limits.

### irccd/plugin.h

- When a loader create a new plugin, it should first call `irc_plugin_init` to
  setup default values.
- The `irc_plugin` should now be initialized by loaders using
  `irc_plugin_set_info` which will copy the appropriate strings.

### irccd/rule.h

The module implements criteria using NULL-terminated list of strings, use the
appropriate `irc_rule_<add|remove>_<criterion>()` functions to update them.

### irccd/server.h

The module has been widely simplified and no longer use inner struct types,
however because it requires several parameters to be set before use new
functions are needed.

To create a new server dynamically one should now:

1. Call `irc_server_new`,
2. Set ident information `irc_server_set_ident`,
3. Set connection information `irc_server_set_params`,
4. Set an optional password if needed `irc_server_set_password`,
5. Call `irc_server_connect` (or `irc_bot_server_add` usually).

All of the server fields are now accessible as read-only for convenience.

Build system
------------

CMake has been removed and no CMake configuration package is provided, use
pkg-config files to build against libirccd which is unchanged.

Migrating from 3.x to 4.x
=========================

Irccd
-----

- The `irccd.conf` is now using a custom syntax.
- There are no longer TCP/IP transports. SSL support for transports has been
  removed too. Only clear UNIX socket are available, use file permissions
  instead of a password.

Irccdctl
--------

- There is no longer configuration file because it now use a plain UNIX socket
  to */tmp/irccd.sock* by default (may be changed through the `-s` option).
- Aliases have been removed, please use shell scripts or aliases instead.
- The `watch` command no longer produce JSON output but only the original
  "human" format but may be used for scripts as it is honored through the
  semantic versioning.
- The command `rule-info` has been removed because it is mostly the same as
  `rule-list`.
- The command `server-mode` signature has changed because it was mostly unusable
  before.

Platform support
----------------

Windows support is now completely obsolescent because the code make excessive
use of POSIX APIs.

Network API
-----------

The network protocol no longer uses JSON but a plain text syntax with one line
per command.

Example:

    SERVER-MESSAGE freenode #staff hello world
    SERVER-LIST

Javascript API
--------------

### Module Irccd

- The property `Irccd.version` and their properties have been renamed to
  `CamelCase` for consistency with other constants from other modules.

### Module File

- The method `File.readline` is no longer marked as slow.
- Methods `File.lines`, `File.read`, `File.readline` and `File.seek`,  now throw
  an exception if the file was closed.

### Module Chrono

- The `Chrono.elapsed()` method is now a property named `elapsed`.
- All methods have been removed except `Chrono.reset`.

### Module Util

- The method `Util.ticks` as been removed.
- The method `Util.cut` now throws a `RangeError` exception if the number of
  lines exceed `maxl` argument instead of returning null.

### Module Server

- The property `channels` in the object returned from `Server.info` is now an
  array of objects which also contain a list of nicknames present in the
  channel.
- The property `channels` in the object for the `Server` constructor now takes
  an array of objects containing two properties each: `name` and `password`
  which must be string (password is optional).
- The property `commandChar` which is provided in both the `Server` constructor
  and the `Server.info` returned object has been renamed to `prefix`.
- The event `onMode` now takes four arguments: server, channel, mode and list
  of arguments to the mode. The previous signature was mostly unusable.
- The method `Server.mode` has a different signature because it was mostly
  unusable.

Plugins
-------

**logger**

- Due to the `onMode` change the template `mode` no longer takes `limit`,
  `user` and `mask` but a string `args` instead.

Migrating from 2.x to 3.x
=========================

Irccd
-----

- Long options have been removed.
- The option `reconnect-tries` has been removed from `[server]` section, use
  `auto-reconnect` boolean option instead,
- The option `reconnect-timeout` has been renamed to `auto-reconnect-delay`.
- The section `[identity]` has been removed, instead move those values inside
  each server in their `[server]` section.
- The section `[format]` and their respective plugin counterparts are renamed to
  `[templates]`.

Irccdctl
--------

- Long options have been removed.
- The functions `server-cnotice` and `server-cmode` have been removed, use
  `server-notice` and `server-mode` instead,
- The option `connect.host` has been renamed to `connect.hostname`,
- The output style has been unified,
- Options `-S` in server-connect have been removed, also
  the port option is now specified with `-p` instead of a positional argument.
- Connection options are now order dependant and must be set before the command
  name.

Plugins
-------

### Logger

- The keyword `source` has been removed and replaced by `channel`,
- The keyword `origin` has been added,
- Formats `cnotice`, `cmode`, `query` have been removed.
- The option `path` has been renamed to `file`.

Network API
-----------

- The requests `server-cnotice` and `server-cmode` have been removed, use
  `server-notice` and `server-mode` instead,
- The request `server-mode` command requires a new argument `channel`.
- The property `host` in request `server-connect` has been renamed to
  `hostname`,
- The request `server-info` sends `hostname` property instead of `host`,
- The event `onWhois` sends `hostname` property instead of `host`,

CMake options
-------------

- All options are now starting with `IRCCD_` for better compatibility with
  external projects,
- CMake now use GNUInstallDirs module to specify installation paths, all
  IRCCD\_WITH\_ options have been replaced by CMAKE\_INSTALL\_ equivalents.

Directories
-----------

- The default plugins path has been changed from **share/irccd/plugins** to
  **lib(arch)/irccd**.

Javascript API
--------------

### Events

- The events `onChannelMode` and `onChannelNotice` have been removed, plugins
  must use `Server.isSelf(target)` to determine a channel/private message,
- The event `onNotice` takes a new `channel` argument,
- The event `onMode` takes new `channel`, `limit`, `user`, `mask` arguments,
- The object information in `onWhois` event now has `hostname` property instead
  of `host`.

### Module Chrono

- The module `ElapsedTimer` has been renamed to `Chrono`,
- The method `Chrono.restart` has been renamed to `Chrono.resume` to reduce
  ambiguity,
- The method `Chrono.reset` has been removed, just use `Chrono.start` instead
  when you want to start accumulate time again.

### Module Directory

- The property `Directory.count` has been removed.

### Module Server

- The methods `Server.cmode` and `Server.cnotice` have been removed, use
  `Server.mode` and `Server.notice` instead,
- The method `Server.mode` requires a new argument `channel`,
- The object returned in the method `Server.info` now has a `hostname` property
  instead of `host`.
- The property `host` in constructor `Server` has been renamed to
  `hostname`,
- The property `sslVerify` in `Server` constructor has been removed.

### Module Plugin

The following properties in `Irccd.Plugin` has been renamed:

- **cachePath** renamed to **paths.cache**,
- **configPath** renamed to **paths.config**,
- **dataPath** renamed to **paths.data**.

Note: these paths are no more automatically detected and set with the new
      `[paths]` and `[paths.<name>]` sections.

### Module Server

- The property `host` in the function `Irccd.Server` has been renamed to
  `hostname`.

### Module System

- The function `Irccd.System.name` has now well defined return value.
