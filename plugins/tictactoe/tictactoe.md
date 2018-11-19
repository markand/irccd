# Tictactoe plugin

This plugin let you play tictactoe over IRC.

Warning: this plugin is verbose.

## Installation

The plugin **tictactoe** is distributed with irccd. To enable it add the following
to your `plugins` section:

```ini
[plugins]
tictactoe = ""
```

## Usage

Execute **tictactoe** plugin with the target opponent nickname. Then each player
send a message in the form **x y** where x targets the column and y the row.

To verify target opponent, this plugins first requests the names on the channel
to ensures a valid player.

If one of the players leaves the channel (either by kick or part) the game is
aborted.

```nohighlight
markand: !tictactoe francis
irccd:   a b c
irccd: 1 . . .
irccd: 2 . . .
irccd: 3 . . .
irccd: markand, it's your turn
```

And then, placing tokens.

```nohighlight
20:27 < markand> a 1
20:27 < irccd>   a b c
20:27 < irccd> 1 x . .
20:27 < irccd> 2 . . .
20:27 < irccd> 3 . . .
20:27 < irccd> francis, it's your turn
20:27 <@francis> c 1
20:27 < irccd>   a b c
20:27 < irccd> 1 x . o
20:27 < irccd> 2 . . .
20:27 < irccd> 3 . . .
20:27 < irccd> markand, it's your turn
20:27 < markand> a 2
20:27 < irccd>   a b c
20:27 < irccd> 1 x . o
20:27 < irccd> 2 x . .
20:27 < irccd> 3 . . .
20:27 < irccd> francis, it's your turn
20:27 <@francis> c 3
20:27 < irccd>   a b c
20:27 < irccd> 1 x . o
20:27 < irccd> 2 x . .
20:27 < irccd> 3 . . o
20:27 < irccd> markand, it's your turn
20:27 < markand> a 3
20:27 < irccd>   a b c
20:27 < irccd> 1 x . o
20:27 < irccd> 2 x . .
20:27 < irccd> 3 x . o
20:27 < irccd> francis, it's your turn
20:27 < irccd> markand, congratulations, you won!
```

## Formats

The **tictactoe** plugin supports the following formats in `[format.tictactoe]`
section:

- draw: when the game ended with no winner,
- invalid: the opponent does not exist or is not valid,
- running: the game is already running,
- turn: message sent when current player change,
- used: the cell requested is already used,
- win: game ended with a winner.

### Keywords supported

The following keywords are supported:

| Format  | Keywords                                   | Notes       |
|---------|--------------------------------------------|-------------|
| (any)   | channel, command, nickname, plugin, server | all formats |
| invalid | origin                                     |             |
| running | origin                                     |             |
