#
# libunity.mk -- embedded unity test framework
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

LIBUNITY := extern/libunity/libunity.a

LIBUNITY_SRCS := extern/libunity/unity.c
LIBUNITY_OBJS := $(LIBUNITY_SRCS:.c=.o)
LIBUNITY_DEPS := $(LIBUNITY_SRCS:.c=.d)

LIBUNITY_CFLAGS += -Iextern/libunity

$(LIBUNITY): $(LIBUNITY_OBJS)
$(LIBUNITY_OBJS): private CFLAGS := $(LIBUNITY_CFLAGS)

all:: $(LIBUNITY)

clean::
	rm -f $(LIBUNITY) $(LIBUNITY_DEPS) $(LIBUNITY_OBJS)

-include $(LIBUNITY_DEPS)
