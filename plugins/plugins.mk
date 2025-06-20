#
# plugins.mk -- official irccd plugins
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
