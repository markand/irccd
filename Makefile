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

MAJOR=                  4
MINOR=                  0
PATCH=                  0

IRCCD=                  irccd/irccd
IRCCD_SRCS=             irccd/main.c
IRCCD_OBJS=             ${IRCCD_SRCS:.c=.o}
IRCCD_DEPS=             ${IRCCD_SRCS:.c=.d}

IRCCDCTL=               irccdctl/irccdctl
IRCCDCTL_SRCS=          irccdctl/main.c
IRCCDCTL_OBJS=          ${IRCCDCTL_SRCS:.c=.o}
IRCCDCTL_DEPS=          ${IRCCDCTL_SRCS:.c=.d}

LIBCOMPAT=              extern/libcompat/libirccd-compat.a

ifeq (${WITH_JS},yes)
LIBDUKTAPE=             extern/libduktape/libirccd-duktape.a
endif

LIBIRCCD=               lib/libirccd.a
LIBIRCCD_SRCS=          lib/irccd/dl-plugin.c
LIBIRCCD_SRCS+=         lib/irccd/irccd.c
LIBIRCCD_SRCS+=         lib/irccd/log.c
LIBIRCCD_SRCS+=         lib/irccd/peer.c
LIBIRCCD_SRCS+=         lib/irccd/plugin.c
LIBIRCCD_SRCS+=         lib/irccd/rule.c
LIBIRCCD_SRCS+=         lib/irccd/server.c
LIBIRCCD_SRCS+=         lib/irccd/subst.c
LIBIRCCD_SRCS+=         lib/irccd/transport.c
LIBIRCCD_SRCS+=         lib/irccd/util.c

ifeq (${WITH_JS},yes)
LIBIRCCD_SRCS+=         lib/irccd/js-plugin.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-chrono.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-file.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-irccd.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-logger.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-plugin.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-server.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-system.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-timer.c
LIBIRCCD_SRCS+=         lib/irccd/jsapi-unicode.c
LIBIRCCD_SRCS+=         lib/irccd/unicode.c
endif

LIBIRCCD_OBJS=          ${LIBIRCCD_SRCS:.c=.o}
LIBIRCCD_DEPS=          ${LIBIRCCD_SRCS:.c=.d}

TESTS=                  tests/test-dl-plugin.c
TESTS+=                 tests/test-log.c
TESTS+=                 tests/test-rule.c
TESTS+=                 tests/test-subst.c
TESTS+=                 tests/test-util.c
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

ifeq (${WITH_SSL},yes)
LIBS+=                  -l ssl -l crypto
endif

all: ${IRCCD} ${IRCCDCTL}

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

ifneq (${WITH_JS},yes)
EXTRA_SEDS+=    -e "/IRCCD_WITH_JS/d"
endif

ifneq (${WITH_SSL},yes)
EXTRA_SEDS+=    -e "/IRCCD_WITH_SSL/d"
endif

lib/irccd/config.h: lib/irccd/config.h.in Makefile config.mk
	sed -e "s/@IRCCD_VERSION_MAJOR@/${MAJOR}/" \
	    -e "s/@IRCCD_VERSION_MINOR@/${MINOR}/" \
	    -e "s/@IRCCD_VERSION_PATCH@/${PATCH}/" \
	    ${EXTRA_SEDS} < $< > $@

${LIBIRCCD_OBJS}: ${LIBCOMPAT} lib/irccd/config.h

${LIBIRCCD}: ${LIBIRCCD_OBJS} ${LIBDUKTAPE}
	${CMD.ar}

${IRCCD}: ${IRCCD_OBJS} ${LIBCOMPAT} ${LIBDUKTAPE} ${LIBIRCCD}
	${CMD.ccld}

${IRCCDCTL}: ${IRCCDCTL_OBJS}
	${CMD.ccld}

# Unit tests.
tests/test-%: tests/test-%.c
	${CC} ${DEFINES} ${INCS} ${CFLAGS} -o $@ $< ${LIBS} ${LDFLAGS}

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
