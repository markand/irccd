#!/bin/sh
#
# sample-hook.sh -- a sample hook in shell script
#
# Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

onConnect()
{
	echo "event:   connect"
	echo "server:  $2"
}

onDisconnect()
{
	echo "event:   disconnect"
	echo "server:  $2"
}

onInvite()
{
	echo "event:   invite"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "target:  $5"
}

onJoin()
{
	echo "event:   join"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
}

onKick()
{
	echo "event:   kick"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "target:  $5"
	echo "reason:  $6"
}

onMessage()
{
	echo "event:   message"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "message: $5"
}

onMe()
{
	echo "event:   me"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "message: $5"
}

onMode()
{
	echo "event:   mode"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "mode:    $5"
	echo "limit:   $6"
	echo "user:    $7"
	echo "mask:    $8"
}

onNick()
{
	echo "event:   nick"
	echo "server:  $2"
	echo "origin:  $3"
	echo "nick:    $4"
}

onNotice()
{
	echo "event:   notice"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "message: $5"
}

onPart()
{
	echo "event:   part"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "reason:  $5"
}

onTopic()
{
	echo "event:   topic"
	echo "server:  $2"
	echo "origin:  $3"
	echo "channel: $4"
	echo "topic:   $5"
}

#
# Call the appropriate function with the same name as the event and pass
# arguments.
#
# Please keep quotes between $@ as some arguments are quoted (like messages).
#
$1 "$@"
