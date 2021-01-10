#!/bin/sh

prog=$(basename $0)
top=$(dirname $0)
compile=0
src=$top/src/null.c

usage()
{
	echo "usage: $prog [-c] function" 1>&2
	exit 1
}

code()
{
	cat <<-EOF
	void $1();

	int
	main(void)
	{
		$1();
	}
	EOF
}

var()
{
	printf "$1" | tr "[:lower:]" "[:upper:]"
}

while getopts "c" opt; do
	case $opt in
	c)
		compile=1 ;;
	*)
		usage ;;
	esac
done

shift $((OPTIND - 1))

if [ $# -eq 0 ]; then
	usage
fi

trap "rm -f test-$1 test-$1.c" QUIT INT TERM EXIT

code $1 > test-$1.c

if [ $compile -eq 0 ]; then
	printf "Checking for function $1: "
fi

if ! $CC $CFLAGS -o test-$1 test-$1.c >/dev/null 2>&1; then
	if [ $compile -eq 0 ]; then
		printf "no\n"
	fi

	src=$top/src/$1.c
	touch $top/src/$1.h
else
	if [ $compile -eq 0 ]; then
		printf "yes\n"
	fi

	printf "#define COMPAT_HAVE_%s\n" $(var $1) > $top/src/$1.h
fi

if [ $compile -eq 1 ]; then
	$CC $CFLAGS -I $top -c $src -o $top/src/$1.o
fi
