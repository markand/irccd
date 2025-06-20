#
# irccd.mk -- main irccd executable
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

IRCCD := irccd/irccd

IRCCD_SRCS := irccd/conf.c
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

$(IRCCD): $(IRCCD_OBJS) $(LIBIRCCD)
$(IRCCD): private LDLIBS += $(LIBIRCCD_LDFLAGS)

$(IRCCD_OBJS): private CFLAGS += $(LIBIRCCD_CFLAGS)

ifeq ($(JS), 1)
$(IRCCD): $(LIBDUKTAPE)
$(IRCCD): private LDLIBS += $(LIBDUKTAPE_LDFLAGS)
$(IRCCD_OBJS): private CFLAGS += $(LIBDUKTAPE_CFLAGS)
endif

ifeq ($(HTTP), 1)
$(IRCCD): private LDLIBS += $(LIBCURL_LDFLAGS)
$(IRCCD_OBJS): private CFLAGS += $(LIBCURL_CFLAGS)
endif

irccd/conf.o irccd/lex.o: private CFLAGS += -D_POSIX_C_SOURCE=202405L -Wno-unused-function -Wno-unneeded-internal-declaration

all:: $(IRCCD)

clean::
	rm -f $(IRCCD) $(IRCCD_DEPS) $(IRCCD_OBJS) irccd/conf.c irccd/conf.h irccd/lex.c

-include $(IRCCD_DEPS)
