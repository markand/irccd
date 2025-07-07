#
# lib.mk -- irccd public API
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

LIBIRCCD := lib/libirccd.a

LIBIRCCD_SRCS := lib/irccd/channel.c
LIBIRCCD_SRCS += lib/irccd/event.c
LIBIRCCD_SRCS += lib/irccd/hook.c
LIBIRCCD_SRCS += lib/irccd/irccd.c
LIBIRCCD_SRCS += lib/irccd/log.c
LIBIRCCD_SRCS += lib/irccd/plugin.c
LIBIRCCD_SRCS += lib/irccd/rule.c
LIBIRCCD_SRCS += lib/irccd/server.c
LIBIRCCD_SRCS += lib/irccd/subst.c
LIBIRCCD_SRCS += lib/irccd/util.c

LIBIRCCD_OBJS := $(LIBIRCCD_SRCS:.c=.o)
LIBIRCCD_DEPS := $(LIBIRCCD_SRCS:.c=.d)

LIBIRCCD_CFLAGS := $(LIBEV_CFLAGS)
LIBIRCCD_CFLAGS += $(LIBUTLIST_CFLAGS)
LIBIRCCD_CFLAGS += $(LIBBSD_CFLAGS)
LIBIRCCD_CFLAGS += -Ilib

LIBIRCCD_LDFLAGS += $(LIBEV)
LIBIRCCD_LDFLAGS += $(LIBUTLIST_LDFLAGS)
LIBIRCCD_LDFLAGS += $(LIBBSD_LDFLAGS)

ifeq ($(SSL), 1)
LIBIRCCD_CFLAGS += $(LIBSSL_CFLAGS)
LIBIRCCD_LDFLAGS += $(LIBSSL_LDFLAGS)
endif

lib/irccd/config.h: lib/irccd/config.h.in
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

$(LIBIRCCD): $(LIBIRCCD_OBJS) | $(LIBEV)
$(LIBIRCCD_SRCS): lib/irccd/config.h
$(LIBIRCCD_OBJS): private CFLAGS += $(LIBBSD_CFLAGS) $(LIBEV_CFLAGS) $(LIBUTLIST_CFLAGS)

all:: $(LIBIRCCD)

clean::
	rm -f $(LIBIRCCD) $(LIBIRCCD_DEPS) $(LIBIRCCD_OBJS) lib/irccd/config.h

install::
	mkdir -p $(DESTDIR)$(INCLUDEDIR)/irccd
	cp lib/irccd/*.h $(DESTDIR)$(INCLUDEDIR)/irccd
	mkdir -p $(DESTDIR)$(LIBDIR)/share/pkgconfig
	sed \
		-e "s,@INCLUDEDIR@,$(INCLUDEDIR),g" \
		-e "s,@VERSION@,$(VERSION),g" \
		< lib/irccd.pc.in > $(LIBDIR)/share/pkgconfig/irccd.pc

-include $(LIBIRCCD_DEPS)
