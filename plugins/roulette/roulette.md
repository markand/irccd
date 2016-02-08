---
title: "Roulette plugin"
header: "Roulette plugin"
---

The plugin **roulette** is a funny script that let you do a russian roulette game but without any injuries.

## Installation

The plugin **roulette** is distributed with irccd. To enable it add the following to your `plugins` section:

````ini
[plugins]
roulette = ""
````

## Usage

The plugin **roulette** just reacts to the special command.

Example:

    markand: !roulette
    irccd: markand, you're lucky this time
    markand: !roulette
    irccd: markand, you're lucky this time
    markand: !roulette
    markand was kicked by irccd [markand, HEADSHOT]

## Configuration

The **roulette** plugin can be configured to use different formats.

The following options are available under the `[plugin.roulette]` section:

  - **format-lucky**: (string) the text to show on luck,
  - **format-shot**: (string) the text to show on shot.

### Keywords supported

The following keywords are supported:

| Format           | Keywords                          | Notes                             |
|------------------|-----------------------------------|-----------------------------------|
| (any)            | server, channel, nickname, origin | all formats                       |
| **format-lucky** | count                             | the number of cylinder count left |

Example:

````ini
[plugin.roulette]
format-lucky = "#{nickname} you're gonna get shot"
format-shot = "BIM"
````
