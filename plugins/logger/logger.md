---
title: "Logger plugin"
header: "Logger plugin"
guide: yes
---

The plugin **logger** may be used to log everything you want. It supports the
following events:

  - onJoin,
  - onKick,
  - onMe,
  - onMessage,
  - onMode,
  - onNotice,
  - onPart,
  - onTopic.

## Installation

The plugin **logger** is distributed with irccd. To enable it add the following
to your `plugins` section:

```ini
[plugins]
logger = ""
```

## Usage

There is nothing to do, except configuring it.

## Configuration

The following options are available under the `[plugin.logger]` section:

  - **path**: (string) the path to the file where to store logs,

## Formats

The **logger** plugin supports the following formats in `[format.logger]`
section:

  - **join**: (string) format when someone joins a channel,
  - **kick**: (string) format when someone has been kicked,
  - **me**: (string) format for emote actions,
  - **message**: (string) format for channel messages,
  - **mode**: (string) format for user mode change,
  - **notice**: (string) format on private notices,
  - **part**: (string) format when someone leaves a channel,
  - **topic**: (string) format when a topic is changed.

### Keywords supported

The following keywords are supported:

| Format      | Keywords                          | Notes                       |
|-------------|-----------------------------------|-----------------------------|
| (any)       | channel, nickname, origin, server | channel may be a nickname   |
| **kick**    | reason, target                    |                             |
| **me**      | message                           | message is the emote action |
| **message** | message                           |                             |
| **mode**    | mode, limit, user, mask           | the mode and its arguments  |
| **notice**  | message                           | the notice message          |
| **part**    | reason                            |                             |
| **topic**   | topic                             |                             |

Example:

<div class="panel panel-info">
 <div class="panel-heading">~/.config/irccd/irccd.conf</div>
 <div class="panel-body">
```ini
[plugin.logger]
path = "/var/log/irccd/#{server}/%y/%m/%d/#{channel}.txt"

[format.logger]
join = "user #{nickname} joined #{channel}"
```
 </div>
</div>
