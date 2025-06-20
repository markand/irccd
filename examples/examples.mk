CONFIGS := examples/irccd.conf.sample

ASSETS := examples/sample-hook.sh
ASSETS += examples/sample-plugin.c
ASSETS += examples/sample-plugin.js

install::
	mkdir -p $(DESTDIR)$(SYSCONFDIR)
	cp $(CONFIGS) $(DESTDIR)$(SYSCONFDIR)
	mkdir -p $(DESTDIR)$(DATADIR)/irccd
	cp $(ASSETS) $(DESTDIR)$(DATADIR)/irccd
