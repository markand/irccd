---
title: "Hangman plugin"
header: "Hangman plugin"
---

Hangman is a plugin to play the hangman game.

## Installation

The plugin **hangman** is distributed with irccd. To enable it add the following to your `plugins` section:

````ini
[plugins]
hangman = ""
````

## Usage

The **hangman** plugin starts when a user execute its special command with no arguments.

````
markand: !hangman
irccd: markand, the game is started, the word to find is: _ _ _ _ _ _ _ _ _ _ _
````

### Proposal

There are two ways for proposing a response to the game. You can either just ask for a letter or for a whole word.

#### By letter

When asking a letter, the message must just be one letter.

````nohighlight
markand: c
irccd: markand, nice! the word is now c _ _ _ _
jean: k
irccd: markand, there is no 'k'.
````

#### By full word

When asking by a word, just put one word as command.

````nohighlight
markand: !hangman couch
irccd: markand, this is not the word.
jean: !hangman candy
irccd: markand, congratulations, the word is candy.
````

## Configuration

The **hangman** plugin can be configured to show different message and to specify a different database file. The default
database file is **CONFDIR/plugin/hangman/words.conf**.

The following options are available under the `[plugin.hangman]` section:

  - **collaborative**: (bool) set to true to enable collaborative mode, a player can't propose two consecutives proposals (Optional, default: true),
  - **file**: (string) the path to the database file,
  - **format-asked**: (string) when a letter has been already asked but present in the word,
  - **format-dead**: (string) when the man was hung,
  - **format-found**: (string) when a correct letter has been placed,
  - **format-running**: (string) when a game is requested but it's already running,
  - **format-start**: (string) when the game starts,
  - **format-win**: (string) when the game succeeded,
  - **format-wrong-word**: (string) when a word proposal is wrong,
  - **format-wrong-letter**: (string) when a letter proposal is wrong.

### Keywords supported

The following keywords are supported:

| Format                  | Keywords                          | Notes                           |
|-------------------------|-----------------------------------|---------------------------------|
| (any)                   | server, channel, nickname, origin | all formats                     |
| **format-asked**        | letter                            | the letter proposal             |
| **format-dead**         | word                              | the word to find                |
| **format-found**        | word                              | the hidden word                 |
| **format-start**        | word                              | the hidden word                 |
| **format-win**          | word                              | the word to find                |
| **format-wrong-word**   | word                              | the invalid word proposal       |
| **format-wrong-letter** | letter                            | the letter proposal             |

Example:

````ini
[plugin.hangman]
format-win = "nice job, the word was #{word}!"
format-wrong-letter = "please try again, there is no #{letter}"
````

## Database file

The database file must contains one word per line.

Example:

````nohighlight
$ cat ~/.config/irccd/plugin/hangman/words.conf
sky
irccd
FreeBSD
door
cat
````
