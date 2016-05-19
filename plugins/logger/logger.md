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

The following options are available under the `[plugin.logger]` section:

  - **path**: (string) the path to the file where to store logs,

**Deprecated in irccd 2.1.0:**

  - **format-cmode**: Use `[format.logger] cmode` instead,
  - **format-cnotice**: Use `[format.logger] cnotice` instead,
  - **format-join**: Use `[format.logger] join` instead,
  - **format-kick**: Use `[format.logger] kick` instead,
  - **format-me**: Use `[format.logger] me` instead,
  - **format-message**: Use `[format.logger] message` instead,
  - **format-mode**: Use `[format.logger] mode` instead,
  - **format-notice**: Use `[format.logger] notice` instead,
  - **format-part**: Use `[format.logger] part` instead,
  - **format-query**: Use `[format.logger] query` instead,
  - **format-topic**: Use `[format.logger] topic` instead,

## Formats

The **logger** plugin supports the following formats in `[format.logger]` section:

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

| Format      | Keywords                          | Notes                           |
|-------------|-----------------------------------|---------------------------------|
| (any)       | nickname, origin, server, source  | source is the channel or nick   |
| **cmode**   | arg, channel, mode,               | the mode and its arguments      |
| **cnotice** | channel, message                  | the message notice              |
| **join**    | channel                           |                                 |
| **kick**    | channel, reason, target           |                                 |
| **me**      | channel, message                  | message is the emote action     |
| **message** | channel, message                  |                                 |
| **mode**    | arg, mode                         | the mode and its arguments      |
| **notice**  | message                           | the notice message              |
| **part**    | channel, reason                   |                                 |
| **query**   | message                           |                                 |
| **topic**   | channel, topic                    |                                 |

The **source** keyword is specially designed to use a generic path for the path parameter.

Example:

<div class="panel panel-info">
 <div class="panel-heading">~/.config/irccd/irccd.conf</div>
 <div class="panel-body">
````ini
[plugin.logger]
path = "/var/log/irccd/#{server}/%y/%m/%d/#{source}.txt"

[format.logger]
join = "user #{nickname} joined #{channel}"
````
 </div>
</div>
