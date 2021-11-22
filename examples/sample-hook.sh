#!/bin/sh
#
# This sample hook is a minimalist logger to show events and their arguments.
#
# Please note that origin is always the fully qualified username with its host
# so you have to split it as shown below.
#
# Those arguments are always available:
#
# $0: hook path
# $1: event name (e.g. onConnect)
# $2: server identifier (as used in the configuration file)
# $3: full origin
#

case $1 in
	#
	# onConnect
	# ---------------------------------------
	# $1: onConnect
	# $2: server-id
	#
	onConnect)
		printf "[%s] connected\n" "$2"
		;;

	#
	# onDisconnect
	# ---------------------------------------
	# $1: onDisconnect
	# $2: server-id
	#
	onDisconnect)
		printf "[%s] connection lost\n" "$2"
		;;

	#
	# onInvite
	# ---------------------------------------
	# $1: onInvite
	# $2: server-id
	# $3: origin
	# $4: channel
	#
	onInvite)
		printf "[%s] %s invited me on %s\n" "$2" "${3%\!*}" "$4"
		;;

	#
	# onJoin
	# ---------------------------------------
	# $1: onJoin
	# $2: server-id
	# $3: origin
	# $4: channel
	#
	onJoin)
		printf "[%s] %s joined %s\n" "$2" "${3%\!*}" "$4"
		;;

	#
	# onMode
	# ---------------------------------------
	# $1: onMode
	# $2: server-id
	# $3: origin
	# $N: arguments...
	#
	onMode)
		printf "[%s] %s changed mode " "$2" "${3%\!*}"
		shift 3
		printf "$@"
		printf "\n"
		;;

	#
	# onMessage
	# ---------------------------------------
	# $1: onMessage
	# $2: server-id
	# $3: origin
	# $4: channel
	# $5: message
	#
	onMessage)
		printf "[%s] (%s) %s: %s\n" "$2" "$4" "${3%\!*}" "$5"
		;;

	#
	# onMe
	# ---------------------------------------
	# $1: onMe
	# $2: server-id
	# $3: origin
	# $4: channel
	# $4: message
	#
	onMe)
		printf "[%s] (%s) * %s %s\n" "$2" "$4" "${3%\!*}" "$5"
		;;

	#
	# onKick
	# ---------------------------------------
	# $1: onKick
	# $2: server-id
	# $3: origin
	# $4: channel
	# $5: target
	# $6: reason
	#
	onKick)
		printf "[%s] %s has been kicked from %s by %s [%s]\n" "$2" "$5" "$4" "${3%\!*}" "$6"
		;;

	#
	# onNick
	# ---------------------------------------
	# $1: onKick
	# $2: server-id
	# $3: origin
	# $4: nickname
	#
	onNick)
		printf "[%s] %s -> %s\n" "$2" "${3%\!*}" "$4"
		;;

	#
	# onNotice
	# ---------------------------------------
	# $1: onNotice
	# $2: server-id
	# $3: origin
	# $4: channel
	# $5: message
	#
	onNotice)
		printf "[%s] (notice) %s %s: %s\n" "$2" "$4" "${3%\!*}" "$5"
		;;

	#
	# onPart
	# ---------------------------------------
	# $1: onPart
	# $2: server-id
	# $3: origin
	# $4: channel
	# $5: reason
	#
	onPart)
		printf "[%s] %s left %s [%s]\n" "$2" "${3%\!*}" "$4" "$5"
		;;

	#
	# onTopic
	# ---------------------------------------
	# $1: onTopic
	# $2: server-id
	# $3: origin
	# $4: channel
	# $5: new topic
	#
	onTopic)
		printf "[%s] %s has changed topic of %s to %s\n" "$2" "${3%\!*}" "$4" "$5"
		;;
esac
