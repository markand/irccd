#
# irccdctl.mk -- irccdctl utility
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

IRCCDCTL := irccdctl/irccdctl

$(IRCCDCTL): $(LIBIRCCD)
$(IRCCDCTL): private CFLAGS += $(LIBIRCCD_CFLAGS)
$(IRCCDCTL): private LDLIBS += $(LIBIRCCD_LDFLAGS)

all:: $(IRCCDCTL)

clean::
	rm -f $(IRCCDCTL) $(IRCCDCTL_DEPS) $(IRCCDCTL_OBJS)

install::
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -p $(IRCCD) $(DESTDIR)$(BINDIR)

-include $(IRCCDCTL_DEPS)
