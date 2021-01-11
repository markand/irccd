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

.POSIX:

.SUFFIXES:
.SUFFIXES: .o .c

include config.mk

IRCCD=          irccd/irccd
IRCCD_SRCS=     extern/libduktape/duktape.c     \
                irccd/log.c                     \
                irccd/server.c                  \
                irccd/subst.c                   \
                irccd/util.c
IRCCD_OBJS=     ${IRCCD_SRCS:.c=.o}
IRCCD_DEPS=     ${IRCCD_SRCS:.c=.d}

TESTS=          tests/test-log.c                \
                tests/test-util.c               \
                tests/test-subst.c
TESTS_OBJS=     ${TESTS:.c=}

FLAGS=          -D_BSD_SOURCE                   \
                -I extern/libduktape            \
                -I extern/libgreatest           \
                -I extern/libcompat/include     \
                -I .

all: ${IRCCD}

.c.o:
	${CC} -MMD ${FLAGS} ${CFLAGS} -c $< -o $@

.c:
	${CC} ${FLAGS} ${CFLAGS} $< -o $@ extern/libcompat/libcompat.a ${IRCCD_OBJS} ${LDFLAGS}

-include ${IRCCD_DEPS}

extern/libcompat/libcompat.a:
	${MAKE} -C extern/libcompat

${IRCCD_OBJS}: extern/libcompat/libcompat.a

${IRCCD}: irccd/main.o ${IRCCD_OBJS}
	${CC} -o $@ extern/libcompat/libcompat.a irccd/main.o ${IRCCD_OBJS} ${LDFLAGS}

clean:
	${MAKE} -C extern/libcompat clean
	rm -f irccd/main.o irccd/main.d ${IRCCD} ${IRCCD_OBJS} ${IRCCD_DEPS}

${TESTS_OBJS}: ${IRCCD_OBJS}

tests: ${TESTS_OBJS}
	for t in ${TESTS_OBJS}; do ./$$t; done

.PHONY: all clean tests
