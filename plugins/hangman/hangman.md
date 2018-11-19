# Hangman plugin

Hangman is a plugin to play the hangman game.

## Installation

The plugin **hangman** is distributed with irccd. To enable it add the following
to your `plugins` section:

```ini
[plugins]
hangman = ""
```

## Usage

The **hangman** plugin starts when a user execute its special command with no
arguments.

```nohighlight
markand: !hangman
irccd: markand, the game is started, the word to find is: _ _ _ _ _ _ _ _ _ _ _
```

If a game is already running, the same command shows the current word.

```nohighlight
markand: !hangman
irccd: markand, the game is already running and the word is: s _ _,
```

### Proposal

There are two ways for proposing a response to the game. You can either just ask
for a letter or for a whole word.

#### By letter

When asking a letter, the message must be one letter.

```nohighlight
markand: c
irccd: markand, nice! the word is now c _ _ _ _
jean: k
irccd: jean, there is no 'k'.
```

#### By full word

When asking by a word, just put one word as command argument.

```nohighlight
markand: !hangman couch
irccd: markand, this is not the word.
jean: !hangman candy
irccd: jean, congratulations, the word is candy.
```

## Configuration

The following options are available under the `[plugin.hangman]` section:

- file: (string) the path to the database file.
- collaborative: (bool) set to true to enable collaborative mode, a player can't
  propose two consecutives proposals (Optional, default: true),

## Formats

The **hangman** plugin supports the following formats in `[format.hangman]`
section:

- asked: (string) when a letter has been already asked but present in the word (Optional),
- dead: (string) when the man was hung (Optional),
- found: (string) when a correct letter has been placed (Optional),
- running: (string) when a game is requested but it's already running (Optional),
- start: (string) when the game starts (Optional),
- win: (string) when the game succeeded (Optional),
- wrong-word: (string) when a word proposal is wrong (Optional),
- wrong-letter: (string) when a letter proposal is wrong (Optional).

### Keywords supported

The following keywords are supported:

| Format                  | Keywords                                           | Notes                           |
|-------------------------|----------------------------------------------------|---------------------------------|
| (any)                   | channel, command, nickname, origin, plugin, server | all formats                     |
| **asked**               | letter                                             | the letter proposal             |
| **dead**                | word                                               | the word to find                |
| **found**               | word                                               | the hidden word                 |
| **start**               | word                                               | the hidden word                 |
| **running**             | word                                               | the hidden word                 |
| **win**                 | word                                               | the word to find                |
| **wrong-word**          | word                                               | the invalid word proposal       |
| **wrong-letter**        | letter                                             | the letter proposal             |

Example:

```ini
[plugin.hangman]
file = "/var/srv/db/words.txt"

[format.hangman]
win = "nice job, the word was #{word}!"
wrong-letter = "please try again, there is no #{letter}"
```

## Database file

The database file must contains one word per line, it must be saved as UTF-8
and words must only contains UTF-8 characters, any other entry will be ignored.

```nohighlight
sky
irccd
FreeBSD
door
cat
```
