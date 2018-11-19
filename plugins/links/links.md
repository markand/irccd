# Links plugin

The plugin **links** is use to analyze links sent on channels. It will load the
web page and extract its title on the same channel.

## Installation

The plugin **links** is distributed with irccd. To enable it add the following
to your `plugins` section:

```ini
[plugins]
links = ""
```

## Usage

The plugin will automatically fetch web page titles on message that contains
either *http://something* or *https://something*.

Example of possible output:

```nohighlight
markand: http://example.org
irccd: Example Domain
```

## Configuration

The following options are available under the `[plugin.links]` section:

- timeout: (int) timeout in seconds before dropping a request (default: 30).

## Formats

The **links** plugin supports the following formats in `[format.links]` section:

- info: message written when title was parsed correctly

### Keywords supported

The following keywords are supported:

| Format | Keywords                                 | Notes                 |
|--------|------------------------------------------|-----------------------|
| info   | channel, nickname, origin, server, title | title is webpage link |
