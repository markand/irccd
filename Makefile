#
# Makefile -- GNU makefile for irccd
#
# Copyright (c) 2013-2025 David Demelier <markand@malikania.fr>
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

CC := clang
PKGCONF := pkg-config
LEX := flex
YACC := bison

-include config.mk

JS ?= 1
HTTP ?= 1
SSL ?= 1
TESTS ?= 1

UID ?= irccd
GID ?= irccd

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

OS := $(shell uname -s)

override CFLAGS += -Wall -Wextra -MMD

%.a:
	$(AR) -rc $@ $^

%.c: %.l
	$(LEX) -o $@ $<

%.c: %.y
	$(YACC) -b tmp -d $<
	mv -f tmp.tab.c $*.c
	mv -f tmp.tab.h $*.h

.PHONY: all
all::

.PHONY: clean
clean::

.PHONY: install
install::

ifeq ($(JS), 1)
include extern/libduktape/libduktape.mk
endif

include extern/libev/libev.mk
include extern/libutlist/libutlist.mk

ifeq ($(TESTS), 1)
include extern/libunity/libunity.mk
endif

include lib/lib.mk
include irccd/irccd.mk
include irccdctl/irccdctl.mk

include plugins/plugins.mk

include examples/examples.mk
include man/man.mk
include systemd/systemd.mk

ifeq ($(TESTS), 1)
include tests/tests.mk
endif
