IRC Client Daemon CHANGES
=========================

irccd current
----------------------

Irccd:

  - The default server identity now uses the system account name if possible by
    default (#607),
  - New sections `[paths]` and `[paths.plugin]` have been added to control
    standard paths for both irccd and plugins (#611),
  - If Mercurial is found, the version is bundled in `irccd --version`.

CMake:

  - CMake no longer create a fake installation directory while building (#674),
  - All targets are placed into the `bin` directory while building (#715).

Network API:

  - Network commands return an error code instead of a string (#739).

Javascript API:

  - The Irccd.Timer API now runs on top of Boost.Asio and no longer have custom
    buggy code (#595),
  - New Irccd.Server.isSelf function (#735).

Internal:

  - The code is now based on Boost for many internal parts of the core, (#593),
    (#594), (#595), (#681), (#697),
  - The libircclient has been replaced by a simple homemade library (#581).

Plugins:

  - Introduce brand new joke plugin (#609).

irccd 2.2.0 2017-09-26
----------------------

  - Add new Irccd.Util.cut function (#635),
  - Add new irccdctl commands to edit rules (#641),
  - Plugin plugin: add options max-list-lines, max-list-columns (#618),
  - Import Duktape 2.1.0 (#648),
  - Fix identity.ctcp-version option (#690).

irccd 2.1.3 2017-07-28
----------------------

  - Rules are now case insensitive (#645),
  - Plugin hangman, history and logger are now case insensitive (#642),
  - Plugin hangman: fix successive word selection (#644).

irccd 2.1.2 2017-06-02
----------------------

  - Fix SSL initialization error in libircclient (#653),
  - Fix various SSL warnings (#652),
  - Fix build on Linux with musl (#649),
  - Fix case sensitivity in hangman and roulette (#642).

irccd 2.1.1 2017-03-07
----------------------

  - Fix invalid documented option transport.family,
  - Fix error when logs.type is set to console,
  - Fix invalid IPV6_V6ONLY option in transports.

irccd 2.1.0 2017-02-01
----------------------

Irccd:

  - Add SSL support in transports,
  - Add authentication support in transports,
  - Fix a warning about daemon on macOS.

Javascript API:

  - New Irccd.File.lines function,
  - Various improvements in Irccd.File API.

Plugins:

  - Add new format section for plugins,
  - Add unit tests for plugins.

Irccdctl:

  - Added brand new plugin-config command,
  - Added aliases,
  - Added unit tests for irccdctl commands.

Libraries:

  - Replaced jansson with Niels Lohmann's JSON library,
  - Updated Duktape to 1.5.1.

Misc:

  - Patterns can now use shell escape sequences,
  - Added .editorconfig file,
  - Split documentation into topics,
  - The code is now split into several individual libraries.

Windows:

  - Get rid of QtIFW and uses NSIS, WIX on Windows,
  - Installer have components,
  - Copy DLL files automatically into fakeroot and install,
  - Added better support for cross-compiling using MinGW.

irccd 2.0.3 2016-11-01
----------------------

  - Fix various errors in logger plugin,
  - Fix quakenet support in auth plugin.

irccd 2.0.2 2016-04-19
----------------------

  - Fix CMake error preventing installation of irccd and irccdctl.

irccd 2.0.1 2016-03-13
----------------------

  - Plugin plugin: fix invalid usage.

irccd 2.0.0 2016-03-01
----------------------

  - Initial 2.0.0 release.
