---
title: "Logger plugin"
header: "Logger plugin"
---

The plugin **logger** may be used to log everything you want. It supports the following events:

  - Channel mode,
  - Channel notice,
  - Join,
  - Kick,
  - Me,
  - Message,
  - Mode,
  - Notice,
  - Part,
  - Query,
  - Topic.

## Installation

The plugin **logger** is distributed with irccd. To enable it add the following to your `plugins` section:

````ini
[plugins]
logger = ""
````

## Usage

There is nothing to do, except configuring it.

## Configuration

The plugin **logger** can be configured to format logs and to use different log path.

The following options are available under the `[plugin.logger]` section:

  - **file**: (string) the path to the file where to store logs,
  - **cmode**: (string) format for channel mode change,
  - **cnotice**: (string) format for channel notices,
  - **join**: (string) format when someone joins a channel,
  - **kick**: (string) format when someone has been kicked,
  - **me**: (string) format for emote actions,
  - **message**: (string) format for channel messages,
  - **mode**: (string) format for user mode change,
  - **notice**: (string) format on private notices,
  - **part**: (string) format when someone leaves a channel,
  - **query**: (string) format on private messages,
  - **topic**: (string) format when a topic is changed.

### Keywords supported

The following keywords are supported:

| Format                  | Keywords                          | Notes                           |
|-------------------------|-----------------------------------|---------------------------------|
| (any)                   | nickname, origin, server, source  | source is the channel or nick   |
| **format-cmode**        | arg, channel, mode,               | the mode and its arguments      |
| **format-cnotice**      | channel, message                  | the message notice              |
| **format-join**         | channel                           |                                 |
| **format-kick**         | channel, reason, target           |                                 |
| **format-me**           | channel, message                  | message is the emote action     |
| **format-message**      | channel, message                  |                                 |
| **format-mode**         | arg, mode                         | the mode and its arguments      |
| **format-notice**       | message                           | the notice message              |
| **format-part**         | channel, reason                   |                                 |
| **format-query**        | message                           |                                 |
| **format-topic**        | channel, topic                    |                                 |

The **source** keyword is specially designed to use a generic path for the path parameter.

Example:

<div class="panel panel-info">
 <div class="panel-heading">~/.config/irccd/irccd.conf</div>
 <div class="panel-body">
````ini
[plugin.logger]
file = "/var/log/irccd/#{server}/%y/%m/%d/#{source}.txt"
format-join = "user #{nickname} joined #{channel}"
````
 </div>
</div>
