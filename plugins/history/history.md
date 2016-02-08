---
title: "History plugin"
header: "History plugin"
---

The plugin **history** is used to check when someone has been seen for the last time on a channel. For that purpose,
irccd needs to be on that channel forever if possible.

## Installation

The plugin **history** is distributed with irccd. To enable it add the following to your `plugins` section:

````ini
[plugins]
history = ""
````

## Usage

The plugin **history** only reacts to the special command. It understands `seen` and `said` sub commands.

  - The sub command `seen` tells you when the user has been seen for the last time.
  - The sub command `said` tells you what the user has said for the last time.

Example:

    markand: !history seen jean
    irccd: markand, the last time I've seen jean was on 18/01/1989 at 20:55
    markand: !history said jean
    irccd: markand, the last thing that jean said was: hello world

## Configuration

You can use different formats.

The following options are available under the `[plugin.history]` section:

  - **path**: (string) path to the JSON file for saving information (Optional, default to cache directory).
  - **format-error**: (string) format when an internal error occured,
  - **format-seen**: (string) format for showing last seen,
  - **format-said**: (string) format for showing the last message,
  - **format-unknown**: (string) format when the user has never been seen,
  - **format-usage**: (strnig) format to show the plugin usage.

### Keywords supported

The following keywords are supported:

| Format                  | Keywords                          | Notes                           |
|-------------------------|-----------------------------------|---------------------------------|
| (any)                   | server, channel, nickname, origin |                                 |
| **path**                | server, channel                   | does not support (any)          |
| **format-seen**         | target, (date)                    | target is the specified nick    |
| **format-said**         | target, message, (date)           |                                 |
| **format-unknown**      | target                            |                                 |

Example:

````ini
[plugin.history]
format-seen = "#{target} has been seen on #{channel} the last time on: %d/%m/%Y %H:%M"
````
