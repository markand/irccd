IRC Client Daemon CHANGES
=========================

irccd 4.1.0 ????-??-??
======================

- Add `irc_bot_pollable_add` to insert a custom interface into the main irccd
  loop.
- A new option `IRCCD_WITH_HTTP` has been added to accommodate the new curl
  dependency for `Irccd.Http` API.
- The plugin `history` manages two different timestamps for message and
  visibility. This helps showing the last message date of someone who is present
  on a channel but idle.

javascript API
--------------

- Brand new asynchronous `Irccd.Http` API.

plugins
-------

- links: rewritten in Javascript using the brand new `Irccd.Http` API.

irccd 4.0.2 2023-01-18
======================

- Fix crash because of undefined variable.

irccd 4.0.1 2022-02-07
======================

- Fix disconnection with multiple servers.
- Fix Javascript `Irccd.Server` constructor function.
- Substitute the irccd version in Javascript plugins.
- Fix `irccdctl plugin-reload` command without argument.

irccd 4.0.0 2022-02-03
======================

This is a major release. See MIGRATING.md file for more information.

The key highlight for this release is the rewrite from C++ to C. The only
runtime dependency required is OpenSSL (if built with SSL support).

irccd
-----

- Irccd keeps track of nicknames in channels by capturing join/part/kick and
  mode changes. It is now more convenient from the plugins to quickly inspect if
  someone is present on a channel.
- It is now possible to change uid/gid of the transport socket file. The file is
  also created with permissions 664.
- A new `paths` command has been added to show default paths.

irccdctl
--------

- Commands `plugin-reload` and `plugin-unload` can be invoked without arguments.
- New `plugin-template` and `plugin-path` command which are synonyms of
  `plugin-config` but for templates and paths respectively.

plugins
-------

- tictactoe: now has a timeout in case of inactivity.

misc
----

- Split irccd-api manual page into individual irccd-api-<module> for a better
  readability.
- New `irccd.conf` and `irccdctl.conf` syntax.

network API
-----------

- Network protocol uses plain text again.
- Transport uses clear UNIX sockets only without passwords.

javascript API
--------------

- Brand new Irccd.Rule API to inspect and manage rules.
- Brand new Irccd.Hook API to inspect and manage hooks.

irccd 3.1.1 2021-01-04
======================

- Synchronize `ping-timeout` option in `[server]` to 1800 seconds by default.
- Enable `auto-reconnect` option in `[server]` by default as specified in the
  manual page.

irccd 3.1.0 2020-07-03
======================

- Added a new hook system. Hooks consist of an alternative approach to plugins
  to extend irccd in any language.

irccd 3.0.3 2019-10-06
======================

- Fix errors in irccdctl.conf example file.
- Add example of password in irccdctl.conf and irccd.conf.

irccd 3.0.2 2019-09-22
======================

- Added *IRCCD_WITH_JS* CMake variable in irccd package.
- Fixed trailing CTCP escape code.
- Fixed invalid parameters in topic event.
- Fixed invalid documentation of Irccd.System.usleep function.
- Fixed invalid system configuration directory.

irccd 3.0.1 2019-09-01
======================

- Fixed an invalid template escape sequence.
- Updated the default configuration files.
- Fix RPATH handling for private libraries like Duktape.

irccd 3.0.0 2019-08-15
======================

This is a major release. See MIGRATING.md file for more information.

irccd
-----

- New sections `[paths]` and `[paths.plugin]` have been added to control
  standard paths for both irccd and plugins.
- Irccd no longer supports uid, gid, pid and daemon features.
- Sections `[identity]` and `[server]` have been merged.
- Local transports support SSL.
- The origin in rule is now first class value.
- New option `ipv4` in `[transport]`.
- New option `ipv4` in `[server]`.
- Section `[format]` is renamed to `[templates]`.
- New commands are available as irccd arguments `info` and `version`.

irccdctl
--------

- New option `ipv4` in `[connect]`.
- New option `-o` in `rule-add`.
- New option `-o` and `-O` in `rule-edit`.

irccd-test
----------

- A brand new `irccd-test` program has been added to tests plugins on the
  command line.

cmake
-----

- CMake no longer create a fake installation directory while building.
- All targets are placed into the `bin` directory while building.

network API
-----------

- Network commands return an error code instead of a string.

javascript API
--------------

- The Irccd.Timer API now runs on top of Boost.Asio and no longer have custom
  buggy code.
- New Irccd.Server.isSelf function.

internal
--------

- The code is now based on Boost for many internal parts of the core.
- The libircclient has been replaced by a simple homemade library.

misc
----

- The documentation is in pure manual pages now.
- All command line options are now in short form only.

plugins
-------

- Introduce brand new joke plugin.
- Introduce brand new tictactoe plugin.
- Introduce brand new links plugin.

irccd 2.2.0 2017-09-26
======================

- Add new Irccd.Util.cut function.
- Add new irccdctl commands to edit rules.
- Plugin plugin: add options max-list-lines, max-list-columns.
- Import Duktape 2.1.0.
- Fix identity.ctcp-version option.

irccd 2.1.3 2017-07-28
======================

- Rules are now case insensitive.
- Plugin hangman, history and logger are now case insensitive.
- Plugin hangman: fix successive word selection.

irccd 2.1.2 2017-06-02
======================

- Fix SSL initialization error in libircclient.
- Fix various SSL warnings.
- Fix build on Linux with musl.
- Fix case sensitivity in hangman and roulette.

irccd 2.1.1 2017-03-07
======================

- Fix invalid documented option transport.family.
- Fix error when logs.type is set to console.
- Fix invalid IPV6\_V6ONLY option in transports.

irccd 2.1.0 2017-02-01
======================

irccd
-----

- Add SSL support in transports.
- Add authentication support in transports.
- Fix a warning about daemon on macOS.

javascript API
--------------

- New Irccd.File.lines function.
- Various improvements in Irccd.File API.

plugins
-------

- Add new format section for plugins.
- Add unit tests for plugins.

irccdctl
--------

- Added brand new plugin-config command.
- Added aliases.
- Added unit tests for irccdctl commands.

libraries
---------

- Replaced jansson with Niels Lohmann's JSON library.
- Updated Duktape to 1.5.1.

misc
----

- Patterns can now use shell escape sequences,
- Added .editorconfig file.
- Split documentation into topics.
- The code is now split into several individual libraries.

windows
-------

- Get rid of QtIFW and uses NSIS, WIX on Windows,
- Installer have components.
- Copy DLL files automatically into fakeroot and install.
- Added better support for cross-compiling using MinGW.

irccd 2.0.3 2016-11-01
======================

- Fix various errors in logger plugin.
- Fix quakenet support in auth plugin.

irccd 2.0.2 2016-04-19
======================

- Fix CMake error preventing installation of irccd and irccdctl.

irccd 2.0.1 2016-03-13
======================

- Plugin plugin: fix invalid usage.

irccd 2.0.0 2016-03-01
======================

This is a major release. See MIGRATING.md file for more information.

The key highlight for this release is the switch from Lua to Javascript. Lua has
lots of drawbacks including poor versioning support which leads to too many
`#ifdef`s in the code base to maintain compatibility over all versions. The
Javascript engine is powered by [Duktape][] and offers proper semantic
versioning.

The Mercurial repository has been reset for this release, revisions for earlier
versions are no longer available.

general
-------

- Long options removed.
- All notions of channel notice and channel mode have been deleted.
- Everywhere applicable, `host` is renamed to `hostname` instead.

irccd
-----

- Section `[identity]` and `[server]` have been merged.
- Section `[format]` and plugin variants have been renamed to `[templates]`.
- New options `uid`, `gid` and `pidfile` in `[general]` section.
- New `[logs]` section with more features.
- New rule system to filter events through their plugins.

irccdctl
--------

- Commands start with a prefix to separate categories (e.g. `plugin-`.
  `server-`, etc).
- New command `watch` to get realtime events.

plugins
-------

- New hangman plugin.
- New plugin plugin.
- Deleted antiflood plugin.
- Deleted date plugin.
- Deleted badwords plugin.

javascript API
--------------

- New `Irccd.ElapsedTimer` API.
- New `Irccd.Timer` API.
- New `Irccd.Unicode` API.
- Deleted Socket API.
- Deleted Thread API.

cmake
-----

- Option start with `IRCCD_` for better grouping in GUIs.
- Use of `GNUInstallDirs` instead of our own macros.

network API
-----------

- Use of JSON message instead of ASCII protocol.

irccd 1.1.5 2015-02-14
======================

- Fix Mac OS X build.
- Fix null constructed strings.
- Fix general.foreground option not working.
- Windows: Lua and OpenSSL are built as DLL and copied to installation.

irccd 1.1.4 2014-03-28
======================

- Fix Visual C++ redistributable installation.

irccd 1.1.3 2014-03-22
======================

- Fix default internet socket address.
- Remove listener disconnection errors,
- Updated Windows C++ 2013 redistributable.

irccd 1.1.2 2014-02-26
======================

- Fix the example in roulette documentation.
- Fix command parsing for onCommand event.

irccd 1.1.1 2014-02-15
======================

- Fix fs.mkdir that didn't return an error.
- Add missing optional mode parameter in fs.mkdir documentation.
- Also add `irccd.VERSION_PATCH`.

irccd 1.1.0 2014-01-30
======================

- Added support for UDP sockets.
- Added a plugin for authentication.
- Windows irccd's home is now the irccd.exe parent directory.
- Added new socket API for Lua.
- Added new thread API for Lua.
- Added support for server reconnection.
- Added support for text formatting with colors and attributes.
- Added support for onMe (CTCP Action) event.
- Added new way to load plugin by paths.
- Server:whois and server:names generate a new events instead of callback.
- Support of connecting and disconnecting at runtime.
- Plugin has more information, getHome() and getName() are deprecated.
- Split irccd.util into irccd.fs and irccd.system.
- Added support for LuaJIT.

irccd 1.0.2 2013-11-01
======================

- Errata, onMe event is not implemented.

irccd 1.0.1 2013-09-17
======================

- Fixed build without Lua.
- Improved documentation a lot.
- Improved NSIS installer.
- Fixed basename() issue.

irccd 1.0.0 2013-09-13
======================

- Initial release.

[Duktape]: http://duktape.org
