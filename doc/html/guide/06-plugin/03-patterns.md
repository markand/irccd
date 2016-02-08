## Common patterns and formatting

Some plugins can be configured, and some of them can use patterns to substitute variables.

### Syntax

The syntax is `?{}` where `?` is replaced by one of the token defined below. Braces are mandatory and cannot be ommited.
To write a literal template construct, prepend the token twice.

### Availables templates

The following templates are available:

  - `%`, date and time (see [Time](#time)),
  - `#{name}`, name will be substituted from the keywords (see [Keywords](#keywords)),
  - `${name}`, name will be substituted from the environment variable (see [Environment variables](#environment-variables)),
  - `@{attributes}`, the attributes will be substituted to IRC colors (see [Attributes](#attributes)),

### Time

When you can use patterns, the date and time may be used just like `strftime(3)` so for the hours and minutes,
you can use **%H:%M**.

### Environment variables

If supported, you can use environment variables like **${HOME}**. Please note that braces are mandatory.

### Attributes

The attribute format is composed of three parts, foreground, background and modifiers, each separated by a comma.

<div class="alert alert-info" role="alert">
**Note**: attributes and colors are not supported by all IRC clients.
</div>

<div class="alert alert-warning" role="alert">
**Warning**: do not use colors and attributes outside IRC (e.g. for storing text in files) because escape codes are
only valid in IRC context.
</div>

#### Available colors

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

#### Available attributes

  - bold,
  - italic,
  - strike,
  - reset,
  - underline,
  - underline2,
  - reverse.

### Keywords

Keywords are arbitrary names that are replaced depending on the context. They are usually available to configure
plugins.

#### Predefined keywords

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

<div class="alert alert-info" role="alert">
**Warning**: these keywords can be overriden by plugins.
</div>

### Examples

#### Valid constructs

  - `#{target}, welcome`: if target is set to "irccd", becomes "irccd, welcome",
  - `@{red}#{target}`: if target is specified, it is written in red.

#### Invalid or literals constructs

  - `##{target}`: will output "#{target}",
  - `##`: will output "##",
  - `#target`: will output "#target",
  - `#{target`: will throw an error.

#### Colors & attributes

  - `@{red,blue}`: will write text red on blue background,
  - `@{default,yellow}`: will write default color text on yellow background,
  - `@{white,black,bold,underline}`: will write white text on black in both bold and underline.

#### In the logger plugin

For instance, using the **logger** plugin, it's possible to customize the pattern to use when someone joins a channel
like that:

````
#{origin} joined #{channel}
````

The keyword **#{origin}** will be substituted to the nickname and **#{channel}** to the channel name.
