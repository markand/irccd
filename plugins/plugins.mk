SED := sed -e "s|@VERSION@|$(VERSION)|g"

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

SED_JS = $(if $(if ifeq ($(PLUGIN_ASK), 1), $(SED) < plugins/ask/ask.js > $(DESTDIR)$(LIBDIR)/irccd/ask.js)

install::
	mkdir -p $(DESTDIR)$(LIBDIR)/irccd
	$(if ifeq ($(PLUGIN_ASK), 1), $(SED) < plugins/ask/ask.js > $(DESTDIR)$(LIBDIR)/irccd/ask.js)
