% irccd-test
% David Demelier
% 2017-12-21

The `irccd-test` program is a simple utility to test plugins on the command
line.

It opens a prompt that waits for user input, each line consist of a specific
plugin event. These are mostly the same as the Javascript API offers.

If compiled with [libedit][] library, the prompt offers basic completion for the
plugin events.

When a event requires a server, a fake debugging server is created if it does
not exists already. That fake server simply prints every command on the command
line instead of sending them through IRC.

# Synopsis

    $ irccd-test [options] plugin-identifier
    $ irccd-test [options] /path/to/plugin

# Options

The following options are available:

  - `-c, --config file`: specify the configuration file.

# Commands

List of available commands:

  - onCommand server origin channel message
  - onConnect server
  - onInvite server origin channel target
  - onJoin server origin channel
  - onKick server origin channel reason
  - onLoad
  - onMe server origin channel message
  - onMessage server origin channel message
  - onMode server origin channel mode limit user mask
  - onNames server channel nick1 nick2 nickN
  - onNick server origin nickname
  - onNotice server origin channel nickname
  - onPart server origin channel reason
  - onReload
  - onTopic server origin channel topic
  - onUnload
  - onWhois server nick user host realname chan1 chan2 chanN

# Example

Example by testing the **ask** plugin.

    $ irccd-test ask
    > onCommand local #test jean will I be rich?
	local: connect
	local: message jean #test, No
	> onCommand local #test jean are you sure?
	local: message jean #test, Yes

As you can see in this example, the first onCommand generates two server
commands, the first connect attempt is being made because irccd-test creates a
new fake server on the fly as **local** was not existing yet. You can ignore
this.

Then, the server sent a message on the **#test** channel and said **No**. The
second onCommand event did not generate a connect event because the local server
was already present. It said on the same server **No** though.

[libedit]: http://thrysoee.dk/editline/
