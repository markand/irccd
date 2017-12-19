---
title: "Joke plugin"
header: "Joke plugin"
guide: yes
---

The plugin **joke** is a convenient command to display jokes in a random order
without displaying always the same.

It loads jokes per channel/server pair and display a unique joke each time it is
invoked.

# Installation

The plugin **joke** is distributed with irccd. To enable it add the following to
your `plugins` section:

```ini
[plugins]
joke = ""
```

## Usage

The plugin **joke** requires a database of jokes file, it consists of a plain
JSON file of array of array of strings.

Example of **jokes.json** file:

```javascript
[
    [
        "Tip to generate a good random password:",
        "Ask a Windows user to quit vim."
    ],
    [
        "Have you tried turning it off and on again?"
    ]
]
```

This file contains two jokes, the first one will be printed on two lines while
the second only has one.

Then, invoke the plugin:

```nohighlight
markand: !joke
irccd: Have you tried turning it off and on again?
markand: !joke
irccd: Tip to generate a good random password:
irccd: Ask a Windows user to quit vim.
```

## Configuration

The following options are available under the `[plugin.history]` section:

  - **file**: (string) path to the JSON jokes files (Optional: defaults to data
              directory/jokes.json)

### Keywords supported

The following keywords are supported:

| Parameter | Keywords        |
|-----------|-----------------|
| **file**  | channel, server |

Warning: if you use keywords in the **file** parameter, you won't have a default
         joke database anymore.

## Formats

The **joke** plugin supports the following formats in `[format.joke]` section:

  - **error**: (string) format when an internal error occured.

### Keywords supported

The following keywords are supported:

| Format    | Keywords                          |
|-----------|-----------------------------------|
| **error** | channel, nickname, origin, server |
