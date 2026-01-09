#
# Makefile -- GNU makefile for irccd
#
# Copyright (c) 2013-2026 David Demelier <markand@malikania.fr>
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

OS := $(shell uname -s)

CC := clang
PKGCONF := pkg-config
LEX := flex
YACC := bison

-include config.mk

GID ?= irccd
HTTP ?= 1
JS ?= 1
MAN ?= 1
SSL ?= 1
TESTS ?= 0
UID ?= irccd

ifeq ($(OS), Linux)
	SYSTEMD ?= 1
endif

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/lib
LOCALSTATEDIR ?= $(PREFIX)/var
MANDIR ?= $(PREFIX)/share/man
SYSCONFDIR ?= $(PREFIX)/etc

LIBBSD_CFLAGS ?= $(shell $(PKGCONF) --cflags libbsd-overlay)
LIBBSD_LDFLAGS ?= $(shell $(PKGCONF) --libs libbsd-overlay)

ifeq ($(HTTP), 1)
LIBCURL_CFLAGS ?= $(shell $(PKGCONF) --cflags libcurl)
LIBCURL_LDFLAGS ?= $(shell $(PKGCONF) --libs libcurl)
endif

ifeq ($(SSL), 1)
LIBSSL_CFLAGS ?= $(shell $(PKGCONF) --cflags openssl)
LIBSSL_LDFLAGS ?= $(shell $(PKGCONF) --libs openssl)
endif

MAJOR := 5
MINOR := 0
PATCH := 0
VERSION := $(MAJOR).$(MINOR).$(PATCH)

# Determine gcc/clang.
CC_IS_GCC        := $(shell $(CC) --version | head -n1 | grep -qE '(gcc|gnu)' && echo 1)
CC_IS_CLANG      := $(shell $(CC) -E -dM -xc /dev/null | grep -qE '^#define __clang__ 1' && echo 1)

# Some warnings.
ifeq ($(CC_IS_GCC), 1)
override CFLAGS += -Wno-format-truncation
endif
ifeq ($(CC_IS_CLANG), 1)
override CFLAGS += -Wno-c23-extensions -Wno-unused-function
endif

# {{{ default rules & targets

override CFLAGS += -Wall -Wextra -MMD

%.a:
	$(AR) -rc $@ $^

%.c: %.l
	$(LEX) -o $@ $<

%.c: %.y
	$(YACC) --output=$*.c --header=$*.h $<

.PHONY: all
all::

.PHONY: clean
clean::

.PHONY: install
install::

.PHONY: out-of-date
out-of-date:

# Convenient target to regenerate compile_commands.json.
compile_commands.json: out-of-date
	@$(MAKE) --always-make --dry-run all \
		| grep -wE -- "($(CC)|$(HOST_CC))" \
		| jq -nR '[inputs|{directory:"$(CURDIR)", command:., file: match("[^ ]+[.]c").string[0:]}]' \
		> $@

# }}}

# {{{ libduktape

ifeq ($(JS), 1)

LIBDUKTAPE := extern/libduktape/libduktape.a

LIBDUKTAPE_SRCS := extern/libduktape/duktape.c
LIBDUKTAPE_OBJS := $(LIBDUKTAPE_SRCS:.c=.o)
LIBDUKTAPE_DEPS := $(LIBDUKTAPE_SRCS:.c=.d)

LIBDUKTAPE_CFLAGS += -Iextern/libduktape
LIBDUKTAPE_LDFLAGS += -lm

$(LIBDUKTAPE): $(LIBDUKTAPE_OBJS)
$(LIBDUKTAPE_OBJS): private override CFLAGS := $(LIBDUKTAPE_CFLAGS)

all:: $(LIBDUKTAPE)

clean::
	rm -f $(LIBDUKTAPE) $(LIBDUKTAPE_DEPS) $(LIBDUKTAPE_OBJS)

-include $(LIBDUKTAPE_DEPS)

endif

# }}}

# {{{ libev

LIBEV_DIR = extern/libev

LIBEV_SRCS += extern/libev/ev.c
LIBEV_OBJS := $(LIBEV_SRCS:.c=.o)
LIBEV_DEPS := $(LIBEV_SRCS:.c=.d)

LIBEV_CFLAGS += -DEV_CHECK_ENABLE=1
LIBEV_CFLAGS += -DEV_EMBED_ENABLE=0
LIBEV_CFLAGS += -DEV_IDLE_ENABLE=0
LIBEV_CFLAGS += -DEV_IO_ENABLE=1
LIBEV_CFLAGS += -DEV_MULTIPLICITY=0
LIBEV_CFLAGS += -DEV_NO_SMP=1
LIBEV_CFLAGS += -DEV_NO_THREADS=1
LIBEV_CFLAGS += -DEV_PERIODIC_ENABLE=0
LIBEV_CFLAGS += -DEV_PREPARE_ENABLE=1
LIBEV_CFLAGS += -DEV_STANDALONE=1
LIBEV_CFLAGS += -DEV_STAT_ENABLE=0
LIBEV_CFLAGS += -DEV_TIMER_ENABLE=1
LIBEV_CFLAGS += -DEV_USE_CLOCK_SYSCALL=1
LIBEV_CFLAGS += -DEV_USE_FLOOR=1
LIBEV_CFLAGS += -DEV_USE_IOURING=0
LIBEV_CFLAGS += -DEV_USE_LINUXAIO=0
LIBEV_CFLAGS += -DEV_USE_MONOTONIC=1
LIBEV_CFLAGS += -DEV_USE_NANOSLEEP=1
LIBEV_CFLAGS += -DEV_USE_POLL=1
LIBEV_CFLAGS += -DEV_USE_REALTIME=1
LIBEV_CFLAGS += -DEV_USE_SELECT=1

ifeq ($(OS), Linux)
LIBEV_CFLAGS += -DEV_USE_EPOLL=1
LIBEV_CFLAGS += -DEV_USE_EVENTFD=1
LIBEV_CFLAGS += -DEV_USE_SIGNALFD=1
LIBEV_CFLAGS += -DEV_USE_TIMERFD=1
endif

LIBEV_CFLAGS += -I$(LIBEV_DIR)

LIBEV = $(LIBEV_DIR)/libev.a
LIBEV_STATIC = $(LIBEV)

$(LIBEV): $(LIBEV_OBJS)
$(LIBEV_OBJS): private override CFLAGS := $(LIBEV_CFLAGS) -Wno-unused-value

all:: $(LIBEV)

clean::
	rm -f $(LIBEV) $(LIBEV_DEPS) $(LIBEV_OBJS)

-include $(LIBEV_DEPS)

# }}}

# {{{ libminicoro

LIBMINICORO_DIR = extern/libminicoro
LIBMINICORO = $(LIBMINICORO_DIR)/libminicoro.a

LIBMINICORO_SRCS = $(LIBMINICORO_DIR)/minicoro.c

LIBMINICORO_OBJS = $(LIBMINICORO_SRCS:.c=.o)
LIBMINICORO_DEPS = $(LIBMINICORO_SRCS:.c=.d)

LIBMINICORO_CFLAGS += -I$(LIBMINICORO_DIR)

LIBMINICORO_STATIC = $(LIBMINICORO)

$(LIBMINICORO): $(LIBMINICORO_OBJS)
$(LIBMINICORO_OBJS): private override CFLAGS += $(LIBMINICORO_CFLAGS)

all:: $(LIBMINICORO)

clean::
	rm -f $(LIBMINICORO) $(LIBMINICORO_DEPS) $(LIBMINICORO_OBJS)

-include $(LIBMINICORO_DEPS)

# }}}

# {{{ libutlist

LIBUTLIST_CFLAGS := -Iextern/libutlist

# }}}

# {{{ libunity

LIBUNITY := extern/libunity/libunity.a

LIBUNITY_SRCS := extern/libunity/unity.c
LIBUNITY_OBJS := $(LIBUNITY_SRCS:.c=.o)
LIBUNITY_DEPS := $(LIBUNITY_SRCS:.c=.d)

LIBUNITY_CFLAGS += -Iextern/libunity

$(LIBUNITY): $(LIBUNITY_OBJS)
$(LIBUNITY_OBJS): private override CFLAGS := $(LIBUNITY_CFLAGS)

all:: $(LIBUNITY)

clean::
	rm -f $(LIBUNITY) $(LIBUNITY_DEPS) $(LIBUNITY_OBJS)

-include $(LIBUNITY_DEPS)

# }}}

# {{{ libcoro

LIBCORO_DIR = extern/libcoro

LIBCORO_SRCS += $(LIBCORO_DIR)/coro/cio.c
LIBCORO_SRCS += $(LIBCORO_DIR)/coro/coro.c
LIBCORO_SRCS += $(LIBCORO_DIR)/coro/ctimer.c

LIBCORO_OBJS = $(LIBCORO_SRCS:.c=.o)
LIBCORO_DEPS = $(LIBCORO_SRCS:.c=.d)

LIBCORO_CFLAGS += $(LIBMINICORO_CFLAGS)
LIBCORO_CFLAGS += $(LIBEV_CFLAGS)
LIBCORO_CFLAGS += -I$(LIBCORO_DIR)

LIBCORO_LDFLAGS += $(LIBMINICORO_LDFLAGS)
LIBCORO_LDFLAGS += $(LIBEV_LDFLAGS)

LIBCORO = $(LIBCORO_DIR)/libcoro.a
LIBCORO_STATIC = $(LIBCORO) $(LIBEV_STATIC) $(LIBMINICORO_STATIC)

$(LIBCORO): $(LIBCORO_OBJS)
$(LIBCORO_OBJS): private override CFLAGS += $(LIBCORO_CFLAGS)

all:: $(LIBCORO)

clean::
	rm -f $(LIBCORO) $(LIBCORO_DEPS) $(LIBCORO_OBJS)

-include $(LIBCORO_DEPS)

# }}}

# {{{ libirccd

LIBIRCCD_DIR = lib

LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/channel.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/event.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/hook.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/irccd.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/log.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/plugin.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/rule.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/server.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/subst.c
LIBIRCCD_SRCS += $(LIBIRCCD_DIR)/irccd/util.c

LIBIRCCD_OBJS = $(LIBIRCCD_SRCS:.c=.o)
LIBIRCCD_DEPS = $(LIBIRCCD_SRCS:.c=.d)

LIBIRCCD_CFLAGS += $(LIBBSD_CFLAGS)
LIBIRCCD_CFLAGS += $(LIBUTLIST_CFLAGS)
LIBIRCCD_CFLAGS += $(LIBCORO_CFLAGS)
LIBIRCCD_CFLAGS += -I$(LIBIRCCD_DIR)

LIBIRCCD_LDFLAGS += $(LIBBSD_LDFLAGS)
LIBIRCCD_LDFLAGS += $(LIBUTLIST_LDFLAGS)
LIBIRCCD_LDFLAGS += $(LIBCORO_LDFLAGS)

ifeq ($(SSL), 1)
LIBIRCCD_CFLAGS += $(LIBSSL_CFLAGS)
LIBIRCCD_LDFLAGS += $(LIBSSL_LDFLAGS)
endif

$(LIBIRCCD_DIR)/irccd/config.h: $(LIBIRCCD_DIR)/irccd/config.h.in
	sed \
		-e "s,@HTTP@,$(HTTP),g" \
		-e "s,@JS@,$(JS),g" \
		-e "s,@LIBDIR@,$(LIBDIR),g" \
		-e "s,@MAJOR@,$(MAJOR),g" \
		-e "s,@MINOR@,$(MINOR),g" \
		-e "s,@PATCH@,$(PATCH),g" \
		-e "s,@SHAREDIR@,$(SHAREDIR),g" \
		-e "s,@SSL@,$(SSL),g" \
		-e "s,@SYSCONFDIR@,$(SYSCONFDIR),g" \
		-e "s,@VARDIR@,$(VARDIR),g" \
		< $< > $@

LIBIRCCD = $(LIBIRCCD_DIR)/libirccd.a
LIBIRCCD_STATIC = $(LIBIRCCD) $(LIBCORO_STATIC)

$(LIBIRCCD): $(LIBIRCCD_OBJS)
$(LIBIRCCD_SRCS): $(LIBIRCCD_DIR)/irccd/config.h
$(LIBIRCCD_OBJS): private override CFLAGS += $(LIBIRCCD_CFLAGS)

all:: $(LIBIRCCD)

clean::
	rm -f $(LIBIRCCD) $(LIBIRCCD_DEPS) $(LIBIRCCD_OBJS) lib/irccd/config.h

install::
	mkdir -p $(DESTDIR)$(INCLUDEDIR)/irccd
	cp $(LIBIRCCD_DIR)/irccd/*.h $(DESTDIR)$(INCLUDEDIR)/irccd
	mkdir -p $(DESTDIR)$(LIBDIR)/share/pkgconfig
	sed \
		-e "s,@INCLUDEDIR@,$(INCLUDEDIR),g" \
		-e "s,@VERSION@,$(VERSION),g" \
		< lib/irccd.pc.in > $(DESTDIR)$(LIBDIR)/share/pkgconfig/irccd.pc

-include $(LIBIRCCD_DEPS)

# }}}

# {{{ irccd

IRCCD = irccd/irccd

IRCCD_SRCS += irccd/conf.c
IRCCD_SRCS += irccd/dl-plugin.c
IRCCD_SRCS += irccd/irccd.c
IRCCD_SRCS += irccd/lex.c
IRCCD_SRCS += irccd/peer.c
IRCCD_SRCS += irccd/transport.c

ifeq ($(JS), 1)
	IRCCD_SRCS += irccd/js-plugin.c
	IRCCD_SRCS += irccd/jsapi-chrono.c
	IRCCD_SRCS += irccd/jsapi-directory.c
	IRCCD_SRCS += irccd/jsapi-file.c
	IRCCD_SRCS += irccd/jsapi-hook.c
	IRCCD_SRCS += irccd/jsapi-http.c
	IRCCD_SRCS += irccd/jsapi-irccd.c
	IRCCD_SRCS += irccd/jsapi-logger.c
	IRCCD_SRCS += irccd/jsapi-plugin.c
	IRCCD_SRCS += irccd/jsapi-rule.c
	IRCCD_SRCS += irccd/jsapi-server.c
	IRCCD_SRCS += irccd/jsapi-system.c
	IRCCD_SRCS += irccd/jsapi-timer.c
	IRCCD_SRCS += irccd/jsapi-unicode.c
	IRCCD_SRCS += irccd/jsapi-util.c
	IRCCD_SRCS += irccd/unicode.c
endif

IRCCD_OBJS := $(IRCCD_SRCS:.c=.o)
IRCCD_DEPS := $(IRCCD_SRCS:.c=.d)

$(IRCCD): $(IRCCD_OBJS) $(LIBIRCCD_STATIC)
$(IRCCD): private override LDLIBS += $(LIBIRCCD_LDFLAGS)

$(IRCCD_OBJS): | $(LIBIRCCD)
$(IRCCD_OBJS): private override CFLAGS += $(LIBIRCCD_CFLAGS)

ifeq ($(JS), 1)
$(IRCCD): $(LIBDUKTAPE)
$(IRCCD): private override LDLIBS += $(LIBDUKTAPE_LDFLAGS)
$(IRCCD_OBJS): private override CFLAGS += $(LIBDUKTAPE_CFLAGS)
endif

ifeq ($(HTTP), 1)
$(IRCCD): private override LDLIBS += $(LIBCURL_LDFLAGS)
$(IRCCD_OBJS): private override CFLAGS += $(LIBCURL_CFLAGS)
endif

ifeq ($(SSL), 1)
$(IRCCD): private override LDLIBS += $(LIBSSL_LDFLAGS)
$(IRCCD_OBJS): private override CFLAGS += $(LIBSSL_CFLAGS)
endif

irccd/conf.o irccd/lex.o: private override CFLAGS += -Wno-unused-function -Wno-unneeded-internal-declaration

all:: $(IRCCD)

clean::
	rm -f $(IRCCD) $(IRCCD_DEPS) $(IRCCD_OBJS) irccd/conf.c irccd/conf.h irccd/lex.c

install::
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -p $(IRCCD) $(DESTDIR)$(BINDIR)

-include $(IRCCD_DEPS)

# }}}

# {{{ irccdctl

IRCCDCTL := irccdctl/irccdctl

$(IRCCDCTL): $(LIBIRCCD)
$(IRCCDCTL): private override CFLAGS += $(LIBIRCCD_CFLAGS)
$(IRCCDCTL): private override LDLIBS += $(LIBIRCCD_LDFLAGS)

all:: $(IRCCDCTL)

clean::
	rm -f $(IRCCDCTL) $(IRCCDCTL_DEPS) $(IRCCDCTL_OBJS)

install::
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -p $(IRCCD) $(DESTDIR)$(BINDIR)

-include $(IRCCDCTL_DEPS)

# }}}

# {{{ plugins

ifeq ($(JS), 1)
	PLUGIN_ASK ?= 1
	PLUGIN_AUTH ?= 1
	PLUGIN_HANGMAN ?= 1
	PLUGIN_HISTORY ?= 1
	PLUGIN_JOKE ?= 1
	PLUGIN_LINKS ?= 1
	PLUGIN_LOGGER ?= 1
	PLUGIN_PLUGIN ?= 1
	PLUGIN_ROULETTE ?= 1
	PLUGIN_TICTACTOE ?= 1
endif

define sed
sed \
	-e "s|@VERSION@|$(VERSION)|g" \
	-e "s|@SYSCONFDIR@|$(SYSCONFDIR)|g"
endef

define install-js-plugin =
$(sed) < plugins/$(1)/$(1).js > $(DESTDIR)$(LIBDIR)/irccd/$(1).js
$(sed) < plugins/$(1)/$(1).7 > $(DESTDIR)$(MANDIR)/man7/irccd-plugin-$(1).7
endef

install::
	mkdir -p $(DESTDIR)$(LIBDIR)/irccd
	mkdir -p $(DESTDIR)$(MANDIR)/man7
ifeq ($(JS), 1)
ifeq ($(PLUGIN_ASK), 1)
	$(call install-js-plugin,ask)
endif
ifeq ($(PLUGIN_AUTH), 1)
	$(call install-js-plugin,auth)
endif
ifeq ($(PLUGIN_HANGMAN), 1)
	$(call install-js-plugin,hangman)
endif
ifeq ($(PLUGIN_HISTORY), 1)
	$(call install-js-plugin,history)
endif
ifeq ($(PLUGIN_JOKE), 1)
	$(call install-js-plugin,joke)
endif
ifeq ($(HTTP), 1)
ifeq ($(PLUGIN_LINKS), 1)
	$(call install-js-plugin,links)
endif
endif
ifeq ($(PLUGIN_LOGGER), 1)
	$(call install-js-plugin,logger)
endif
ifeq ($(PLUGIN_PLUGIN), 1)
	$(call install-js-plugin,plugin)
endif
ifeq ($(PLUGIN_ROULETTE), 1)
	$(call install-js-plugin,roulette)
endif
ifeq ($(PLUGIN_TICTACTOE), 1)
	$(call install-js-plugin,tictactoe)
endif
endif

# }}}

# {{{ examples

CONFIGS := examples/irccd.conf.sample

ASSETS := examples/sample-hook.sh
ASSETS += examples/sample-plugin.c

ifeq ($(JS), 1)
ASSETS += examples/sample-plugin.js
endif

install::
	mkdir -p $(DESTDIR)$(SYSCONFDIR)
	cp $(CONFIGS) $(DESTDIR)$(SYSCONFDIR)
	mkdir -p $(DESTDIR)$(DATADIR)/irccd
	cp $(ASSETS) $(DESTDIR)$(DATADIR)/irccd

# }}}

# {{{ man

ifeq ($(MAN), 1)

MAN_DATE := June 18, 2025
MAN_SUBST := sed -e "s|@IRCCD_MAN_DATE@|$(MAN_DATE)|g"

install::
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	mkdir -p $(DESTDIR)$(MANDIR)/man5
	$(MAN_SUBST) < man/irccd.1 > $(DESTDIR)$(MANDIR)/man1/irccd.1
	$(MAN_SUBST) < man/irccd.conf.5 > $(DESTDIR)$(MANDIR)/man5/irccd.conf.5
	$(MAN_SUBST) < man/irccdctl.1 > $(DESTDIR)$(MANDIR)/man1/irccdctl.1

endif

# }}}

# {{{ systemd

ifeq ($(SYSTEMD), 1)

install::
	mkdir -p $(DESTDIR)$(LIBDIR)/systemd/system
	sed \
		-e "s,@UID@,$(UID),g" \
		-e "s,@GID@,$(GID),g" \
		-e "s,@PATH@,$(BINDIR)/irccd",g \
		< systemd/irccd.service.in \
		> $(DESTDIR)$(LIBDIR)/systemd/system/irccd.service

endif

# }}}

# {{{ tests

ifeq ($(TESTS), 1)

#
# We don't link to irccd library directly because we mock some of the modules
# for the tests.
#

TESTS_LIB_SRCS := lib/irccd/channel.c
TESTS_LIB_SRCS += lib/irccd/hook.c
TESTS_LIB_SRCS += lib/irccd/irccd.c
TESTS_LIB_SRCS += lib/irccd/log.c
TESTS_LIB_SRCS += lib/irccd/plugin.c
TESTS_LIB_SRCS += lib/irccd/rule.c
TESTS_LIB_SRCS += lib/irccd/subst.c
TESTS_LIB_SRCS += lib/irccd/util.c

TESTS_LIB_SRCS += irccd/dl-plugin.c
TESTS_LIB_SRCS += irccd/unicode.c

ifeq ($(JS), 1)
	TESTS_LIB_SRCS += irccd/js-plugin.c
	TESTS_LIB_SRCS += irccd/jsapi-chrono.c
	TESTS_LIB_SRCS += irccd/jsapi-directory.c
	TESTS_LIB_SRCS += irccd/jsapi-file.c
	TESTS_LIB_SRCS += irccd/jsapi-hook.c

	ifeq ($(fdef HTTP), 1)
		TESTS_LIB_SRCS += irccd/jsapi-http.c
	endif

	TESTS_LIB_SRCS += irccd/jsapi-irccd.c
	TESTS_LIB_SRCS += irccd/jsapi-logger.c
	TESTS_LIB_SRCS += irccd/jsapi-plugin.c
	TESTS_LIB_SRCS += irccd/jsapi-rule.c
	TESTS_LIB_SRCS += irccd/jsapi-server.c
	TESTS_LIB_SRCS += irccd/jsapi-system.c
	TESTS_LIB_SRCS += irccd/jsapi-timer.c
	TESTS_LIB_SRCS += irccd/jsapi-unicode.c
	TESTS_LIB_SRCS += irccd/jsapi-util.c
endif

TESTS_LIB_SRCS += tests/mock/server.c

TESTS_LIB_OBJS := $(TESTS_LIB_SRCS:.c=.o)
TESTS_LIB_DEPS := $(TESTS_LIB_SRCS:.c=.d)

TESTS := tests/test-bot
TESTS += tests/test-channel
TESTS += tests/test-dl-plugin
TESTS += tests/test-event
TESTS += tests/test-rule
TESTS += tests/test-subst
TESTS += tests/test-util

ifeq ($(JS), 1)
	TESTS += tests/test-jsapi-chrono
	TESTS += tests/test-jsapi-directory
	TESTS += tests/test-jsapi-file
	TESTS += tests/test-jsapi-irccd
	TESTS += tests/test-jsapi-system
	TESTS += tests/test-jsapi-timer
	TESTS += tests/test-jsapi-unicode
	TESTS += tests/test-jsapi-util
	TESTS += tests/test-plugin-ask
	TESTS += tests/test-plugin-auth
	TESTS += tests/test-plugin-hangman
	TESTS += tests/test-plugin-history
	TESTS += tests/test-plugin-joke
	TESTS += tests/test-plugin-logger
	TESTS += tests/test-plugin-plugin
	TESTS += tests/test-plugin-tictactoe
endif

TESTS_DEPS := $(addsuffix .d,$(TESTS))

TESTS_CFLAGS := $(LIBUNITY_CFLAGS)
TESTS_CFLAGS += $(LIBIRCCD_CFLAGS)
TESTS_CFLAGS += -DTOP=\"$(CURDIR)\"
TESTS_CFLAGS += -DIRCCD_EXECUTABLE=\"$(IRCCD)\"
TESTS_CFLAGS += -I.

TESTS_LDFLAGS := $(LIBIRCCD_LDFLAGS)

$(TESTS): $(TESTS_LIB_OBJS) $(LIBUNITY) | $(IRCCD)

ifeq ($(JS), 1)
TESTS_CFLAGS += $(LIBDUKTAPE_CFLAGS)
TESTS_LDFLAGS += $(LIBDUKTAPE_LDFLAGS)

$(TESTS): $(LIBDUKTAPE)
endif

ifeq ($(HTTP), 1)
TESTS_CFLAGS += $(LIBCURL_CFLAGS)
TESTS_LDFLAGS += $(LIBCURL_LDFLAGS)
endif

$(TESTS): private override CFLAGS += $(TESTS_CFLAGS)
$(TESTS): private override LDLIBS += $(TESTS_LDFLAGS)

# special test file that export its own symbols.
ifeq ($(OS), Linux)
tests/test-dl-plugin: private override LDFLAGS += -Wl,-E
endif

$(TESTS_LIB_OBJS): private override CFLAGS += $(LIBIRCCD_CFLAGS)

all:: $(TESTS)

clean::
	rm -f $(TESTS_LIB_OBJS) $(TESTS_LIB_DEPS)
	rm -f $(TESTS) $(TESTS_DEPS) $(TESTS_OBJS)

.PHONY:
tests: $(TESTS)
	for t in $^; do ./$$t; done

-include $(TESTS_LIB_DEPS)

endif

# }}}
