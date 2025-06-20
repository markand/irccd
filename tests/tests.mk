#
# tests.mk -- test files
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

#
# We don't like to irccd library directly because we mock some of the modules
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

ifeq ($(HTTP), 1)
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

$(TESTS): private CFLAGS += $(TESTS_CFLAGS)
$(TESTS): private LDLIBS += $(TESTS_LDFLAGS)

# special test file that export its own symbols.
tests/test-dl-plugin: private LDFLAGS += -Wl,-E

$(TESTS_LIB_OBJS): private CFLAGS += $(LIBIRCCD_CFLAGS)

all:: $(TESTS)

clean::
	rm -f $(TESTS_LIB_OBJS) $(TESTS_LIB_DEPS)
	rm -f $(TESTS) $(TESTS_DEPS) $(TESTS_OBJS)

.PHONY:
tests: $(TESTS)
	for t in $^; do ./$$t; done

-include $(TESTS_LIB_DEPS)
