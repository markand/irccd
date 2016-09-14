---
function: info
js: true
summary: "Get information about a plugin."
synopsis: "Irccd.Plugin.info(name)"
arguments:
  - "**name**: the plugin identifier, if not specified the current plugin is selected."
returns: "The plugin information or undefined if the plugin was not found."
---

The returned object as the following properties:

  - **name**: (string) the plugin identifier,
  - **author**: (string) the author,
  - **license**: (string) the license,
  - **summary**: (string) a short description,
  - **version**: (string) the version
