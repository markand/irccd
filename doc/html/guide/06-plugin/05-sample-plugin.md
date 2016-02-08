## Your first plugin

Now, we will write a simple plugin that repeat every sentences said on a
channel, it's quite easy but you should not use it on a real channel or you'll
probably get kicked.

### Requirements

First of all, you need to be a little familiar with the JavaScript syntax.

### Create your plugin

We will create our new plugin under the home user plugin path. If you're running
on Unix systems it is usually **${XDG_CONFIG_HOME}/.local/share/irccd/plugins**.

So let start by creating a plugin named **repeater.js**. On my system the file
will live as **/home/markand/.local/share/irccd/plugins/repeater.js**.

### First event

#### Registering the callback

Remember, plugins are made through the event driven mechanism, so we must define a function that will be called when
a user said something on the channel. The function defined on channel message is called `onMessage`.

It has the following signature:

````javascript
function onMessage(server, origin, channel, message)
````

The parameters are defined as following:

- **server**, on which server the message happened
- **origin**, who emit the message (full nickname with host)
- **channel**, on which channel
- **message**, and the message content

#### Send the response

Now that we have the message, the channel and the server, we can send the copy of the message. For this, you must take
care of the server parameter. The server is one of defined in the server section from the configuration file.

There are several methods available for the object, they are defined in the [Irccd.Server][server-api] documentation.

But the on we are interested in is [Server.prototype.message][server-message]. This function takes two parameters, the
target which can be a nickname or a channel and the message.

So the only thing to do is the following:

````javascript
function onMessage(server, origin, channel, message)
{
    server.message(channel, message)
}
````

That's it!

You've just made a brand new plugin, of course it's not a very powerful one but at least you understood the way it
works. With the powerful API provided you will be able to create a bunch of plugins that can fits your needs, such as
a content provider, a moderator, a calculator and so on.

[server-api]: @baseurl@/api/module/Irccd.Server/index.html
[server-message]: @baseurl@/api/module/Irccd.Server/method/message.html
