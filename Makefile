#
# Makefile -- POSIX makefile for irccd
#
# Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
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

.SUFFIXES:
.SUFFIXES: .o .c

include config.mk

IRCCD=                  irccd/irccd
IRCCD_SRCS=             irccd/main.c
IRCCD_OBJS=             ${IRCCD_SRCS:.c=.o}
IRCCD_DEPS=             ${IRCCD_SRCS:.c=.d}

LIBCOMPAT=              extern/libcompat/libirccd-compat.a

ifeq (${WITH_JS},yes)
LIBDUKTAPE=             extern/libduktape/libirccd-duktape.a
endif

LIBIRCCD=               lib/libirccd.a
LIBIRCCD_SRCS=          lib/irccd/dl-plugin.c
LIBIRCCD_SRCS+=         lib/irccd/log.c
LIBIRCCD_SRCS+=         lib/irccd/plugin.c
LIBIRCCD_SRCS+=         lib/irccd/server.c
LIBIRCCD_SRCS+=         lib/irccd/subst.c
LIBIRCCD_SRCS+=         lib/irccd/util.c
LIBIRCCD_OBJS=          ${LIBIRCCD_SRCS:.c=.o}
LIBIRCCD_DEPS=          ${LIBIRCCD_SRCS:.c=.d}

TESTS=                  tests/test-dl-plugin.c
TESTS+=                 tests/test-log.c
TESTS+=                 tests/test-util.c
TESTS+=                 tests/test-subst.c
TESTS_OBJS=             ${TESTS:.c=}

DEFINES=                -D_BSD_SOURCE
DEFINES+=               -DSOURCEDIR=\"`pwd`\"

INCS=                   -I extern/libcompat/include
ifeq (${WITH_JS},yes)
INCS+=                  -I extern/libduktape
endif
INCS+=                  -I extern/libgreatest
INCS+=                  -I lib

LIBS=                   -L extern/libcompat
ifeq (${WITH_JS},yes)
LIBS+=                  -L extern/libduktape
endif
LIBS+=                  -L lib

LIBS+=                  -l irccd-compat
ifeq (${WITH_JS},yes)
LIBS+=                  -l irccd-duktape
endif
LIBS+=                  -l irccd

all: ${IRCCD}

.c.o:
	${CMD.cc}

-include ${LIBIRCCD_DEPS}
-include ${IRCCD_DEPS}

${LIBCOMPAT}:
	${MAKE} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" -C extern/libcompat

ifeq (${WITH_JS},yes)
${LIBDUKTAPE}:
	${MAKE} CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" -C extern/libduktape
endif

${LIBIRCCD_OBJS}: ${LIBCOMPAT}

${LIBIRCCD}: ${LIBIRCCD_OBJS}
	${CMD.ar}

${IRCCD}: ${IRCCD_OBJS} ${LIBCOMPAT} ${LIBDUKTAPE} ${LIBIRCCD}
	${CMD.ccld}

# Unit tests.
tests/test-%.o: tests/test-%.c
	${CMD.cc}
tests/test-%: tests/test-%.o ${LIBCOMPAT} ${IRCCD_OBJS}
	${CMD.ccld}

${TESTS_OBJS}: ${LIBIRCCD}

# Sample plugin for test-dl-plugin.
tests/example-dl-plugin${EXT.shared}: tests/example-dl-plugin.o
	${CMD.ld-shared}

tests/test-dl-plugin: tests/example-dl-plugin${EXT.shared}

tests: ${TESTS_OBJS}
	for t in ${TESTS_OBJS}; do ./$$t; done

clean:
	${MAKE} -C extern/libcompat clean
	${MAKE} -C extern/libduktape clean
	rm -f ${LIBIRCCD} ${LIBIRCCD_OBJS} ${LIBIRCCD_DEPS}
	rm -f ${IRCCD} ${IRCCD_OBJS} ${IRCCD_DEPS}
	rm -f tests/example-dl-plugin${EXT.shared} tests/example-dl-plugin.o tests/example-dl-plugin.d
	rm -f ${TESTS_OBJS}

.PHONY: all clean tests
