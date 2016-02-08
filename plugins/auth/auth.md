---
title: "Auth plugin"
header: "Auth plugin"
---

The plugin **auth** provides generic authentication to the most popular services.

For the moment, **auth** supports the following backends:

  - **nickserv**: the NickServ service, `/msg NickServ identify user pass`
  - **quakenet**: the quakenet.org service, `/msg Q@CServe.quakenet.org AUTH user pass`

## Installation

The plugin **auth** is distributed with irccd. To enable it add the following to your `plugins` section:

````ini
[plugins]
auth = ""
````

## Usage

You must configure the file to enable authentication.

## Configuration

In your **irccd.conf** file, add the `[plugin.auth]` section and fill with the following parameters:

  - **server.type**: (string), must be **nickserv** or **quakenet**,
  - **server.password**: (string), the password,
  - **server.username**: (string), the username to use. Required for **quakenet**, optional for **nickserv**.

You must replace **server** with one defined in a `[server]` section.

Example:

````ini
[plugin.auth]
freenode.type = "nickserv"
freenode.password = "mysecretpassword"
freenode.username = "jeanfrancois"

wanadoo.type = "nickserv"
wanadoo.password = "wanadoo is dead"
````
