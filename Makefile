CC := clang
CFLAGS := -std=c2x
PKGCONF := pkg-config
LEX := flex
YACC := bison

-include config.mk

JS ?= 1
HTTP ?= 1
SSL ?= 1

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
ETCDIR ?= $(PREFIX)/etc
LIBDIR ?= $(PREFIX)/lib
MANDIR ?= $(PREFIX)/share/man
SHAREDIR ?= $(PREFIX)/share
VARDIR ?= $(PREFIX)/var

LIBBSD_CFLAGS ?= $(shell $(PKGCONF) --cflags libbsd-overlay)
LIBBSD_LDFLAGS ?= $(shell $(PKGCONF) --libs libbsd-overlay)

ifeq ($(HTTP), 1)
LIBCURL_CFLAGS ?= $(shell $(PKGCONF) --cflags libcurl)
LIBCURL_LDFLAGS ?= $(shell $(PKGCONF) --libs libcurl)
endif

ifeq ($(SSL), 1)
LIBSLS_CFLAGS ?= $(shell $(PKGCONF) --cflags openssl)
LIBSLS_LDFLAGS ?= $(shell $(PKGCONF) --libs openssl)
endif

MAJOR := 5
MINOR := 0
PATCH := 0

override CFLAGS += -Wall -Wextra -MMD

%.a:
	$(AR) -rc $@ $^

%.c: %.l
	$(LEX) -o $@ $<

%.c: %.y
	$(YACC) --header=$*.h --output=$*.c $<

.PHONY: all
all::

.PHONY: clean
clean::

ifeq ($(JS), 1)
include extern/libduktape/libduktape.mk
endif

include extern/libev/libev.mk
include extern/libutlist/libutlist.mk

include lib/lib.mk
include irccd/irccd.mk
include irccdctl/irccdctl.mk
