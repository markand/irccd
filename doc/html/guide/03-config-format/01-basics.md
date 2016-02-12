# Configuration file format

Both `irccd` and `irccdctl` use configuration file in a extended [INI][ini] format.

## General syntax

The file syntax has following rules:

  1. Each option is stored in a section,
  2. Some sections may be redefined multiple times,
  3. Empty option must have quotes (e.g. `option = ""`).

[ini]: https://en.wikipedia.org/wiki/INI_file
