% irccdctl
% David Demelier
% 2017-12-08

The `irccdctl` utility let you control a running `irccd` instance.

# Synopsis

    usage: irccdctl plugin-config plugin [variable] [value]
           irccdctl plugin-info plugin
           irccdctl plugin-list
           irccdctl plugin-load logger
           irccdctl plugin-reload plugin
           irccdctl plugin-unload plugin
           irccdctl rule-list
           irccdctl server-connect [options] id host [port]
           irccdctl server-disconnect [server]
           irccdctl server-info server
           irccdctl server-invite server nickname channel
           irccdctl server-join server channel [password]
           irccdctl server-kick server target channel [reason]
           irccdctl server-list
           irccdctl server-me server target message
           irccdctl server-message server target message
           irccdctl server-mode server mode
           irccdctl server-nick server nickname
           irccdctl server-notice server target message
           irccdctl server-part server channel [reason]
           irccdctl server-reconnect [server]
           irccdctl server-topic server channel topic
           irccdctl watch [-f|--format json|native]

# Syntax

The general syntax for running an irccdctl command is:

    irccdctl commandname arg1 arg2 arg3 ... argn

# Shell escaping issue

Some shells may discard arguments if they begins with a hash. For instance,
`bash` will not understand the following command:

    $ irccdctl server-join localhost #staff

Instead, enclose the arguments with quotes

    $ irccdctl server-join localhost "#staff"

# Commands

## plugin-config

Get or set a plugin configuration variable.

If both variable and value are provided, sets the plugin configuration to the
respective variable name and value.

If only variable is specified, shows its current value. Otherwise, list all
variables and their values.

### Usage

    $ irccdctl plugin-config plugin [variable] [value]

### Example

    $ irccdctl plugin-config ask

## plugin-info

Get plugin information.

### Usage

    $ irccdctl plugin-info name

### Example

    $ irccdctl plugin-info ask

## plugin-list

Get the list of all loaded plugins.

### Usage

    $ irccdctl plugin-list

## plugin-load

Load a plugin into the irccd instance.

### Usage

    $ irccdctl plugin-load plugin

### Example

$ irccdctl load ask

## plugin-reload

Reload a plugin by calling the appropriate onReload event, the plugin is not
unloaded and must be already loaded.

### Usage

    $ irccdctl plugin-reload name

### Example

    $ irccdctl plugin-reload logger

## plugin-unload

Unload a loaded plugin from the irccd instance.

### Usage

    $ irccdctl plugin-unload name

### Example

    $ irccdctl plugin-unload logger

## rule-add

Add a new rule to irccd.

If no index is specified, the rule is added to the end.

### Usage

    $ irccdctl rule-add [options] accept|drop

Available options:

  - **-c, --add-channel**: match a channel
  - **-e, --add-event**: match an event
  - **-i, --index**: rule position
  - **-p, --add-plugin**: match a plugin
  - **-s, --add-server**: match a server

### Example

    $ irccdctl rule-add -p hangman drop
    $ irccdctl rule-add -s localhost -c #games -p hangman accept

## rule-edit

Edit an existing rule in irccd.

All options can be specified multiple times.

Available options:

  - **a, --action**: set action
  - **c, --add-channel**: match a channel
  - **C, --remove-channel**: remove a channel
  - **e, --add-event**: match an event
  - **E, --remove-event**: remove an event
  - **p, --add-plugin**: match a plugin
  - **P, --add-plugin**: remove a plugin
  - **s, --add-server**: match a server
  - **S, --remove-server**: remove a server

### Usage

    usage: irccdctl rule-edit [options] index

### Example

    $ irccdctl rule-edit -p hangman 0
    $ irccdctl rule-edit -S localhost -c #games -p hangman 1

## rule-info

Show a rule.

### Usage

    $ irccdctl rule-info index

### Example

    $ irccdctl rule-info 0
    $ irccdctl rule-info 1

## rule-list

List all rules.

### Usage

    $ irccdctl rule-list

## rule-move

Move a rule from the given source at the specified destination index.

The rule will replace the existing one at the given destination moving
down every other rules. If destination is greater or equal the number of rules,
the rule is moved to the end.

### Usage

    irccdctl rule-move source destination

### Example

    irccdctl rule-move 0 5
    irccdctl rule-move 4 3

## rule-remove

Remove an existing rule.

### Usage

    $ irccdctl rule-remove index

### Example

    $ irccdctl rule-remove 0
    $ irccdctl rule-remove 1

## server-connect

Connect to a new IRC server.

### Usage

    $ irccdctl server-connect [options] name host port

Available options:

  - **-c, --command**: specify the command char
  - **-n, --nickname**: specify a nickname
  - **-r, --realname**: specify a real name
  - **-S, --ssl-verify**: verify SSL
  - **-s, --ssl**: connect using SSL
  - **-u, --username**: specify a user name

### Example

    $ irccdctl server-connect -n jean example irc.example.org
    $ irccdctl server-connect --ssl example irc.example.org 6697

## server-disconnect

Disconnect from a server.

If server is not specified, irccd disconnects all servers.

### Usage

    $ irccdctl server-disconnect [server]

### Example

    $ irccdctl server-disconnect
    $ irccdctl server-disconnect localhost

## server-invite

Invite the specified target on the channel.

### Usage

    $ irccdctl server-invite server nickname channel

### Example

    $ irccdctl server-invite freenode xorg62 #staff

## server-join

Join the specified channel, the password is optional.

### Usage

    $ irccdctl server-join server channel [password]

### Example

    $ irccdctl server-join freenode #test
    $ irccdctl server-join freenode #private-club secret

## server-kick

Kick the specified target from the channel, the reason is optional.

### Usage

    $ irccdctl server-kick server target channel [reason]

### Example

    $ irccdctl kick freenode jean #staff "Stop flooding"

## server-list

Get the list of all connected servers.

### Usage

    $ irccdctl server-list

## server-me

Send an action emote.

### Usage

    $ irccdctl server-me server target message

### Example

    $ irccdctl server-me freenode #staff "going back soon"

## server-message

Send a message to the specified target or channel.

### Usage

    $ irccdctl server-message server target message

### Example

    $ irccdctl server-message freenode #staff "Hello from irccd"

## server-mode

Set channel or irccd's user mode.

### Usage

    $ server-mode server channel mode [limit] [user] [mask]

### Example

    $ irccdctl server-mode local irccd +i       # set user mode to +i
    $ irccdctl server-mode local #staff +o jean # enable operator for jean

## server-nick

Change irccd's nickname.

### Usage

    $ irccdctl server-nick server nickname

### Example

    $ irccdctl server-nick freenode david

## server-notice

Send a private notice to the specified target.

### Usage

    $ irccdctl server-notice server target message

### Example

    $ irccdctl server-notice freenode jean "I know you are here."

## server-part

Leave the specified channel, the reason is optional.

**Warning**: not all IRC servers support giving a reason to leave a channel, do
             not specify it if this is a concern.

### Usage

    $ irccdctl server-part server channel [reason]

### Example

    $ irccdctl server-part freenode #staff
    $ irccdctl server-part freenode #botwar "too noisy"

## server-reconnect

Force reconnection of one or all servers.

If server is not specified, all servers will try to reconnect.

### Usage

    $ irccdctl server-reconnect [server]

### Example

    $ irccdctl server-reconnect
    $ irccdctl server-reconnect wanadoo

## server-topic

Change the topic of the specified channel.

### Usage

    $ irccdctl server-topic server channel topic

### Example

    $ irccdctl server-topic freenode #wmfs "This is the best channel"

## watch

Start watching irccd events.

You can use different output formats, native is human readable format, json is
pretty formatted json.

### Usage

    $ irccdctl watch [-f|--format native|json]

### Example

    $ irccdctl watch
    $ irccdctl watch -f json
