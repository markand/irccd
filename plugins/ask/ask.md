---
title: "Ask plugin"
header: "Ask plugin"
guide: yes
---

The plugin **ask** is funny script that helps you in your life. It will tells you if you will be rich, famous and so on.

## Installation

The plugin **ask** is distributed with irccd. To enable it add the following to your `plugins` section:

````ini
[plugins]
ask = ""
````

## Usage

The plugin **ask** reacts to special command, you just need to do `!ask the question` and it will give you a response.

## Example

````nohighlight
markand: !ask will I be rich?
irccd: markand, No.
````

## File

By default, **ask** will only answer yes or no. It's possible to add any answers you want in the file **answers.conf** located in the ask
configuration directory.

Just add one line per answer like this:

<div class="panel panel-info">
 <div class="panel-heading">~/.config/irccd/plugin/ask/answers.conf</div>
 <div class="panel-body">
````nohighlight
Stop dreaming,
Definitely sure!
Maybe
````
 </div>
</div>

## Configuration

The following options are available under the `[plugin.ask]` section:

  - **file**: (string) the path to the database file (Optional, default: **answers.conf** in configuration directory).
