---
title: Configuration file format
guide: yes
toc: yes
---

Both `irccd` and `irccdctl` use configuration file in a extended [INI][ini] format.

# General syntax

The file syntax has following rules:

  1. Each option is stored in a section,
  2. Some sections may be redefined multiple times,
  3. Empty option must have quotes (e.g. `option = ""`).

# The @include statement

Irccd adds an extension to this format by adding an `@include` keyword which let you splitting your configuration file.

<div class="alert alert-info" role="alert">
**Note:** this `@include` statement must be at the beginning of the file and must be surrounded by quotes if the file
name has spaces.
</div>

You can use both relative or absolute paths. If relative paths are used, they are relative to the current file being
parsed.

## Example

````ini
@include "rules.conf"
@include "servers.conf"

[mysection]
myoption = "1"
````

# The list construct

When requested, an option can have multiples values in a list. The syntax uses parentheses and values are separated
by commas.

If the list have only one value, you can just use a simple string.

## Examples

<div class="panel panel-success">
 <div class="panel-heading">**Example:** two servers defined in a rule</div>
 <div class="panel-body">
````ini
[rule]
servers = ( "server1", "server2" )
````
 </div>
</div>

<div class="panel panel-success">
 <div class="panel-heading">**Example:** only one server</div>
 <div class="panel-body">
````ini
[rule]
servers = "only-one-server"
````
 </div>
</div>

<div class="alert alert-info" role="alert">
**Note:** spaces are completely optional.
</div>

[ini]: https://en.wikipedia.org/wiki/INI_file
