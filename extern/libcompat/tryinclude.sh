#!/bin/sh

prog=$(basename $0)
top=$(dirname $0)

usage()
{
	echo "usage: $prog include" 1>&2
	exit 1
}

code()
{
	cat <<-EOF
	#include <$1>

	int
	main(void)
	{
	}
	EOF
}

trap "rm -f test-$1 test-$1.c" QUIT INT TERM EXIT

code $1 > test-$1.c

printf "Checking for include file $1: "

if ! $CC $CFLAGS -o test-$1 test-$1.c; then
	printf "no\n"
	touch $top/include/$1
else
	printf "yes\n"
fi
