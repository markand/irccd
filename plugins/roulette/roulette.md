# Roulette plugin

The plugin **roulette** is a funny script that let you do a russian roulette
game but without any injuries.

## Installation

The plugin **roulette** is distributed with irccd. To enable it add the
following to your `plugins` section:

```ini
[plugins]
roulette = ""
```

## Usage

The plugin **roulette** just reacts to the special command.

Example:

```nohighlight
markand: !roulette
irccd: markand, you're lucky this time
markand: !roulette
irccd: markand, you're lucky this time
markand: !roulette
markand was kicked by irccd [markand, HEADSHOT]
```

## Formats

The **roulette** plugin supports the following formats in `[format.roulette]`
section:

- lucky: (string) the text to show on luck,
- shot: (string) the text to show on shot.

### Keywords supported

The following keywords are supported:

| Format    | Keywords                                           | Notes                             |
|-----------|----------------------------------------------------|-----------------------------------|
| (any)     | channel, command, nickname, origin, plugin, server | all formats                       |
| **lucky** | count                                              | the number of cylinder count left |

Example:

```ini
[format.roulette]
lucky = "#{nickname} you're gonna get shot"
shot = "BIM"
```
