---
header: JavaScript plugin introduction
guide: yes
---

Irccd can be extended with JavaScript plugins. This chapter will tell you how
plugins work within irccd and how to create your first plugin.

This chapter covers also some things to do and to avoid in plugins.

# Why JavaScript?

You may wonder why JavaScript was chosen in irccd. Originally, irccd used Lua as
the scripting language but for many reasons, it has been replaced with
Javascript.

However, many aspects between Lua and JavaScript are similar:

  - Both languages are extremly small with very light API,
  - It is easy to sandbox the interpreter for security reasons,
  - It is very easy to implement your own API from C++ code.

The current JavaScript interpreter is powered by [Duktape][duktape].

[duktape]: http://duktape.org

# Paths

Irccd will find plugins in many paths depending on the configuration or the
operating system.

## Unix paths

  1. `${XDG_DATA_HOME}/irccd/plugins`
  2. `${HOME}/.local/share/irccd/plugins`
  3. `/usr/local/share/irccd/plugins`

## Windows paths

  1. `C:\\Users\\YourUser\\irccd\\plugins`
  2. `Path\\To\\Irccd\\Directory\\share\\plugins`

# Plugin metadata

While it's not mandatory, please set the following variables in a global `info`
object:

  - **author**: (string) your name, usually mail address,
  - **license**: (string) an arbitrary license name,
  - **summary**: (string) a short comment about your plugin,
  - **version**: (string) the plugin version.

Example:

````javascript
info = {
    author: "David Demelier <markand@malikania.fr>",
    license: "ISC",
    summary: "A FPS game for IRC",
    version: "0.0.0.0.0.0.1"
};
````

# Plugin creation

Now, we will write a simple plugin that repeat every sentences said on a
channel, it's quite easy but you should not use it on a real channel or you'll
probably get kicked.

We will create our new plugin under the home user plugin path. If you're running
on Unix systems it is usually **${XDG_CONFIG_HOME}/.local/share/irccd/plugins**.

So let start by creating a plugin named **repeater.js**. On my system the file
will live as **/home/markand/.local/share/irccd/plugins/repeater.js**.

# First event

## Registering the callback

Remember, plugins are made through the event driven mechanism, so we must define
a function that will be called when a user said something on the channel. The
function defined on channel message is called [onMessage][].

It has the following signature:

````javascript
function onMessage(server, origin, channel, message)
````

The parameters are defined as following:

  - **server**, on which server the message happened
  - **origin**, who emit the message (full nickname with host)
  - **channel**, on which channel
  - **message**, and the message content

## Send the response

Now that we have the message, the channel and the server, we can send the copy
of the message. For this, you must take care of the server parameter. The server
is one of defined in the server section from the configuration file.

There are several methods available for the object, they are defined in the
[Irccd.Server][server-api] documentation.

But the on we are interested in is [Server.prototype.message][server-message].
This function takes two parameters, the target which can be a nickname or a
hannel and the message.

So the only thing to do is the following:

````javascript
function onMessage(server, origin, channel, message)
{
    server.message(channel, message)
}
````

That's it!

You've just made a brand new plugin, of course it's not a very powerful one but
at least you understood the way it works. With the powerful API provided you
will be able to create a bunch of plugins that can fits your needs, such as
a content provider, a moderator, a calculator and so on.

# Do and do not

There are things which should be avoided if possible.

## Error and load

Since irccd 1.1, one should not write code outside Javascript supported events
functions. Internally, irccd store the plugin information the complete read. If
a plugin has an syntax error or a bad API call, irccd looks for the plugin
metadata information and since it is not currently stored, it generate the
"unitialized state" error.

## Handling error

There are two ways of handling error.

### Your plugin can't continue running

If you need a file, specific resource so your plugin.

If you call the error function while you are in the `onLoad` callback, the
plugin is not added to the registry. However, if the function is called in any
other event, the plugin remains in the list.

### Your plugin has errors but can run

If for instance, the plugin has errors but can still run for any reason. One
should use the `Irccd.Logger` API. The function in this API will write a message
in the irccd output.

**Example**

````javascript
function onCommand()
{
    if (something_is_wrong)
        Irccd.Logger.warning("error condition")
}
````

This will output to the irccd log something like:

````nohighlight
plugin foo: error condition
````

[onMessage]: @baseurl@/api/event/onMessage.html
[server-api]: @baseurl@/api/module/Irccd.Server/index.html
[server-message]: @baseurl@/api/module/Irccd.Server/Irccd.Server.prototype.message.html
