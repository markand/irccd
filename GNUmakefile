#
# GNUmakefile -- GNU make for irccd
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

CC=             cc
CFLAGS=         -Wall -Wextra -Wno-cpp

PREFIX=         /usr/local
BINDIR=         ${PREFIX}/bin
ETCDIR=         ${PREFIX}/etc
INCDIR=         ${PREFIX}/include
LIBDIR=         ${PREFIX}/lib
MANDIR=         ${PREFIX}/share/man
SHAREDIR=       ${PREFIX}/share
VARDIR=         ${PREFIX}/var

SSL=            1
JS=             1

MAJOR=          4
MINOR=          0
PATCH=          0

LIB_SRCS=       lib/irccd/channel.c \
                lib/irccd/conn.c \
                lib/irccd/event.c \
                lib/irccd/hook.c \
                lib/irccd/irccd.c \
                lib/irccd/log.c \
                lib/irccd/plugin.c \
                lib/irccd/rule.c \
                lib/irccd/server.c \
                lib/irccd/subst.c \
                lib/irccd/util.c
LIB_OBJS=       ${LIB_SRCS:.c=.o}
LIB_DEPS=       ${LIB_SRCS:.c=.d}

IRCCD_SRCS=     irccd/conf.c \
                irccd/dl-plugin.c \
                irccd/lex.c \
                irccd/peer.c \
                irccd/transport.c \
                irccd/unicode.c

ifeq (${JS},1)
IRCCD_SRCS+=    extern/libduktape/duktape.c \
                irccd/js-plugin.c \
                irccd/jsapi-chrono.c \
                irccd/jsapi-directory.c \
                irccd/jsapi-file.c \
                irccd/jsapi-hook.c \
                irccd/jsapi-irccd.c \
                irccd/jsapi-logger.c \
                irccd/jsapi-plugin.c \
                irccd/jsapi-rule.c \
                irccd/jsapi-server.c \
                irccd/jsapi-system.c \
                irccd/jsapi-timer.c \
                irccd/jsapi-unicode.c \
                irccd/jsapi-util.c
endif

IRCCD_OBJS=     ${IRCCD_SRCS:.c=.o}
IRCCD_DEPS=     ${IRCCD_SRCS:.c=.d}

MAN1=           man/irccd.1 \
                man/irccdctl.1

MAN3=           man/irccd-api-chrono.3 \
                man/irccd-api-directory.3 \
                man/irccd-api-file.3 \
                man/irccd-api-hook.3 \
                man/irccd-api-logger.3 \
                man/irccd-api-plugin.3 \
                man/irccd-api-rule.3 \
                man/irccd-api-server.3 \
                man/irccd-api-system.3 \
                man/irccd-api-timer.3 \
                man/irccd-api-unicode.3 \
                man/irccd-api-util.3 \
                man/irccd-api.3 \
                man/libirccd-channel.3 \
                man/libirccd-compat.3 \
                man/libirccd-event.3 \
                man/libirccd-hook.3 \
                man/libirccd-log.3 \
                man/libirccd-rule.3 \
                man/libirccd-subst.3 \
                man/libirccd-util.3 \
                man/libirccd.3

MAN5=           man/irccd.conf.5

MAN7=           man/irccd-ipc.7 \
                man/irccd-templates.7

PLUGINS.js=     ask auth hangman history joke logger plugin roulette tictactoe
PLUGINS.c=      links

TESTS=          tests/test-bot.c                \
                tests/test-channel.c            \
                tests/test-dl-plugin.c          \
                tests/test-event.c              \
                tests/test-log.c                \
                tests/test-rule.c               \
                tests/test-subst.c              \
                tests/test-util.c

ifeq (${JS},1)
TESTS+=         tests/test-jsapi-chrono.c       \
                tests/test-jsapi-directory.c    \
                tests/test-jsapi-file.c         \
                tests/test-jsapi-irccd.c        \
                tests/test-jsapi-timer.c        \
                tests/test-jsapi-system.c       \
                tests/test-jsapi-unicode.c      \
                tests/test-jsapi-util.c         \
                tests/test-plugin-ask.c         \
                tests/test-plugin-auth.c        \
                tests/test-plugin-hangman.c     \
                tests/test-plugin-history.c     \
                tests/test-plugin-joke.c        \
                tests/test-plugin-logger.c      \
                tests/test-plugin-plugin.c      \
                tests/test-plugin-tictactoe.c
endif

TESTS_OBJS=     ${TESTS:.c=}

# Per system commands.
OS:=            $(shell uname -s)

# Compile flags.
DEFS=           -D_BSD_SOURCE -DLIBBSD_OVERLAY -DTOP=\"`pwd`\"

ifeq (${DEBUG},1)
CFLAGS+=        -O0 -g
else
CFLAGS+=        -O0 -g
endif

# Include directories.
INCS=           -I lib/
INCS+=          -I ./
INCS+=          -I extern/libgreatest/
INCS+=          -I extern/libketopt/
INCS+=          $(shell pkg-config --cflags libbsd-overlay)

ifeq (${SSL},1)
INCS+=          $(shell pkg-config --cflags openssl)
endif
ifeq (${JS},1)
INCS+=          -I extern/libduktape
endif

# Whole libraries for every binaries.
LIBS+=          -l pthread
LIBS+=          -l m

ifeq (${OS},Linux)
LIBS+=          -l dl
endif

LIBS+=          $(shell pkg-config --libs libbsd-overlay)

ifeq (${SSL},1)
LIBS+=          $(shell pkg-config --libs openssl)
endif

# For config.h file.
ifeq (${SSL},1)
SED.ssl=        s/@define WITH_SSL@/\#define IRCCD_WITH_SSL/
else
SED.ssl=        /@define WITH_SSL@/d

endif
ifeq (${JS},1)
SED.js=         s/@define WITH_JS@/\#define IRCCD_WITH_JS/
else
SED.js=         /@define WITH_JS@/d
endif


ifeq (${OS},Darwin)
SHFLAGS=        -undefined dynamic_lookup
else
SHFLAGS=        -shared
endif

CMD.cc=         ${CC} ${DEFS} ${INCS} ${CFLAGS} -MMD -c $< -o $@
CMD.ccld=       ${CC} ${DEFS} ${INCS} ${CFLAGS} -o $@ $^ ${LIBS} ${LDFLAGS}
CMD.cchost=     ${CC} -Wl,-E -o $@ ${DEFS} ${INCS} ${CFLAGS} \
	-Wl,--whole-archive $^ -Wl,--no-whole-archive ${LIBS} ${LDFLAGS}
CMD.ccplg=      ${CC} ${DEFS} ${INCS} ${CFLAGS} ${SHFLAGS} -o $@ $^ ${LIBS} ${LDFLAGS}

.SUFFIXES:
.SUFFIXES: .c .o .js

# Template for Javascript plugins.
define js-plugin =
PLUGINS.all+=   plugin-${1}
PLUGINS.inst+=  install-plugin-${1}

.PHONY: plugin-${1}
plugin-${1}:

.PHONY: install-plugin-${1}
install-plugin-${1}:
	mkdir -p ${DESTDIR}${LIBDIR}/irccd
	mkdir -p ${DESTDIR}${MANDIR}/man7
	cp plugins/${1}/${1}.js ${DESTDIR}${LIBDIR}/irccd
	cp plugins/${1}/${1}.7 ${DESTDIR}${MANDIR}/man7/irccd-plugin-${1}.7
endef

# Template for C native plugins.
define c-plugin =
PLUGINS.all+=   plugin-${1}
PLUGINS.objs+=  plugins/${1}/${1}.so
PLUGINS.inst+=  install-plugin-${1}

.PHONY: plugin-${1}
plugin-${1}: plugins/${1}/${1}.so

.PHONY: install-plugin-${1}
install-plugin-${1}:
	mkdir -p ${DESTDIR}${LIBDIR}/irccd
	mkdir -p ${DESTDIR}${MANDIR}/man7
	cp plugins/${1}/${1}.so ${DESTDIR}${LIBDIR}/irccd
	cp plugins/${1}/${1}.7 ${DESTDIR}${MANDIR}/man7/irccd-plugin-${1}.7
endef

.c.o:
	${CMD.cc}

.c:
	${CMD.ccld}

all: irccd/irccd irccdctl/irccdctl

-include ${LIB_DEPS} ${IRCCD_DEPS}

lib/irccd/config.h: lib/irccd/config.h.in
	sed -e "s,@ETCDIR@,${ETCDIR},g" \
		-e "s,@LIBDIR@,${LIBDIR},g" \
		-e "s,@SHAREDIR@,${SHAREDIR},g" \
		-e "s,@VARDIR@,${VARDIR},g" \
		-e "s,@MAJOR@,${MAJOR},g" \
		-e "s,@MINOR@,${MINOR},g" \
		-e "s,@PATCH@,${PATCH},g" \
		-e "${SED.ssl}" \
		-e "${SED.js}" \
		< $< > $@

${LIB_OBJS} ${IRCCD_OBJS} irccd/main.o: lib/irccd/config.h

irccd/conf.c: irccd/conf.y
	bison -d -o $@ $<

irccd/lex.c: irccd/lex.l
	flex -o $@ $<

irccd/irccd: irccd/main.o ${IRCCD_OBJS} ${LIB_OBJS}
	${CMD.cchost}

irccdctl/irccdctl: ${LIB_OBJS}

$(foreach p,${PLUGINS.js},$(eval $(call js-plugin,${p})))
$(foreach p,${PLUGINS.c},$(eval $(call c-plugin,${p})))

install:
	mkdir -p ${DESTDIR}${BINDIR}
	cp irccd/irccd ${DESTDIR}${BINDIR}
	chmod 755 ${DESTDIR}${BINDIR}/irccd
	cp irccdctl/irccdctl ${DESTDIR}${BINDIR}
	chmod 755 ${DESTDIR}${BINDIR}/irccdctl
	mkdir -p ${DESTDIR}${MANDIR}/man1
	cp ${MAN1} ${DESTDIR}${MANDIR}/man1
	mkdir -p ${DESTDIR}${MANDIR}/man3
	cp ${MAN3} ${DESTDIR}${MANDIR}/man3
	mkdir -p ${DESTDIR}${MANDIR}/man5
	cp ${MAN5} ${DESTDIR}${MANDIR}/man5
	mkdir -p ${DESTDIR}${MANDIR}/man7
	cp ${MAN7} ${DESTDIR}${MANDIR}/man7
	mkdir -p ${DESTDIR}${ETCDIR}
	cp irccd/irccd.conf ${DESTDIR}${ETCDIR}/irccd.conf.sample
	mkdir -p ${DESTDIR}${LIBDIR}/pkgconfig
	sed -e "s,@MAJOR@,${MAJOR}," \
		-e "s,@MINOR@,${MINOR}," \
		-e "s,@PATCH@,${PATCH}," \
		-e "s,@INCDIR@,${INCDIR}," \
		-e "s,@SHFLAGS@,${SHFLAGS}," \
		< lib/irccd.pc.in > ${DESTDIR}${LIBDIR}/pkgconfig/irccd.pc

install-plugins: ${PLUGINS.inst}

install-systemd:
	mkdir -p ${DESTDIR}${LIBDIR}/systemd/system
	sed -e "s,@PATH@,${BINDIR}/irccd," \
		< systemd/irccd.service \
		> ${DESTDIR}${LIBDIR}/systemd/system/irccd.service

tests/data/example-dl-plugin.so: tests/data/example-dl-plugin.c
	${CMD.ccplg}

${TESTS_OBJS}: ${IRCCD_OBJS} ${LIB_OBJS} | irccd/irccd tests/data/example-dl-plugin.so

# Generic plugin build command.
plugins/%.so: plugins/%.c | ${IRCCD_OBJS}
	${CMD.ccplg}

# Plugin `links` require libcurl.
plugins/links/links.so: plugins/links/links.c ${LIB_OBJS}
	${CMD.ccplg} $(shell pkg-config --libs --cflags libcurl)

plugins: ${PLUGINS.all}

tests/%: tests/%.c
	${CMD.cchost}

tests: ${TESTS_OBJS}
	for t in ${TESTS_OBJS}; do ./$$t -v; done

clean:
	rm -f lib/irccd/config.h ${LIB_OBJS} ${LIB_DEPS}
	rm -f irccd/irccd irccd/main.o irccd/main.d \
		irccd/conf.c irccd/conf.h irccd/lex.c \
		${IRCCD_OBJS} ${IRCCD_DEPS}
	rm -f irccdctl/irccdctl
	rm -f ${TESTS_OBJS} tests/data/example-dl-plugin.so
	rm -f ${PLUGINS.objs}

.PHONY: all clean install install-plugins install-systemd plugins tests
