#!/bin/sh

prog=$(basename $0)
top=$(dirname $0)

usage()
{
	echo "usage: $prog lib" 1>&2
	exit 1
}

code()
{
	cat <<-EOF
	int
	main(void)
	{
	}
	EOF
}

lib()
{
	printf "$1" | sed 's|^lib||;s|\.a||'
}

name=$(lib $1)

trap "rm -f test-$name test-$name.c" QUIT INT TERM EXIT

code > test-$name.c

printf "Checking for library $name: "

if ! $CC $CFLAGS -o test-$name test-$name.c $LDFLAGS -l$name; then
	printf "no\n"
	$CC $CFLAGS -c $top/src/null.c -o $top/src/null.o
	$AR -rc $top/lib/lib$name.a $top/src/null.o
	rm -f $top/src/null.o
else
	printf "yes\n"
fi
