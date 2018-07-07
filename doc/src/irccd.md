% irccd
% David Demelier
% 2017-12-08

The `irccd` program is an IRC bot which connects to one or more severs and
dispatches events to plugins.

# Synopsis

    $ irccd [options...]

# Options

The following options are available:

  - `-c, --config file`: specify the configuration file,
  - `-v, --verbose`: be verbose,
  - `--version`: show the version.

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

# Templates and formatting

Plugins can be configured using a powerful template syntax, this allows editing
the plugin messages to override them.

The syntax is `?{}` where `?` is replaced by one of the token defined below.
Braces are mandatory and cannot be ommited. To write a literal template
construct, prepend the token twice.

## Availables templates

The following templates are available:

  - `%`, date and time (see [Time](#time)),
  - `#{name}`, name will be substituted from the keywords
    (see [Keywords](#keywords)),
  - `${name}`, name will be substituted from the environment variable
    (see [Environment variables](#environment-variables)),
  - `@{attributes}`, the attributes will be substituted to IRC colors
    (see [Attributes](#attributes)),

## Time

When you can use patterns, the date and time may be used just like `strftime(3)`
so for the hours and minutes, you can use **%H:%M**.

## Environment variables

If supported, you can use environment variables like **${HOME}**. Please note
that braces are mandatory.

## IRC attributes

The attribute format is composed of three parts, foreground, background and
modifiers, each separated by a comma.

Note: attributes and colors are not supported by all IRC clients.

Warning: do not use colors and attributes outside IRC (e.g. for storing text in
         files) because escape codes are only valid in IRC context.

### Available colors

  - white,
  - black,
  - blue,
  - green,
  - red,
  - brown,
  - purple,
  - orange,
  - yellow,
  - lightgreen,
  - cyan,
  - lightcyan,
  - lightblue,
  - pink,
  - grey,
  - lightgrey.

### Available attributes

  - bold,
  - italic,
  - strike,
  - reset,
  - underline,
  - underline2,
  - reverse.

## Shell attributes

Like IRC attributes, it's possible to specify colors and attributes in some
places such as logger configuration.

Warning: colors are not supported on all platforms.

### Available colors

  - black,
  - red,
  - green,
  - orange,
  - blue,
  - purple,
  - cyan,
  - white,
  - default.

### Available attributes

  - bold,
  - dim,
  - underline,
  - blink,
  - reverse,
  - hidden.

## Keywords

Keywords are arbitrary names that are replaced depending on the context. They
are usually available to configure plugins.

### Predefined keywords

Here's the list of keywords that a lot of plugins uses:

  - **#{channel}**, the channel name,
  - **#{command}**, the command to invoke the plugin, e.g. `!ask`,
  - **#{message}**, a message (depending on context),
  - **#{origin}**, the full user, e.g. `markand!~mkd@localhost`,
  - **#{nickname}**, the short nickname,
  - **#{plugin}**, the plugin name,
  - **#{server}**, the current server name,
  - **#{topic}**, the topic,
  - **#{target}**, a target, e.g. a person who gets kicked.

Warning: these keywords can be overriden by plugins.

## Examples

### Valid constructs

  - `#{target}, welcome`: if target is set to "irccd", becomes "irccd, welcome",
  - `@{red}#{target}`: if target is specified, it is written in red.

### Invalid or literals constructs

  - `##{target}`: will output "#{target}",
  - `##`: will output "##",
  - `#target`: will output "#target",
  - `#{target`: will throw an error.

### Colors & attributes

  - `@{red,blue}`: will write text red on blue background,
  - `@{default,yellow}`: will write default color text on yellow background,
  - `@{white,black,bold,underline}`: will write white text on black in both bold
    and underline.

### In the logger plugin

For instance, using the **logger** plugin, it's possible to customize the
pattern to use when someone joins a channel like that:

    #{origin} joined #{channel}

The keyword **#{origin}** will be substituted to the nickname and **#{channel}**
to the channel name.

# See also

  - irccd.conf(5)
  - irccdctl(1)
