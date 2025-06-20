DATE := June 18, 2025
SUBST := sed -e "s|@IRCCD_MAN_DATE@|$(DATE)|g"

install::
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	mkdir -p $(DESTDIR)$(MANDIR)/man5
	$(SUBST) < man/irccd.1 > $(DESTDIR)$(MANDIR)/man1/irccd.1
	$(SUBST) < man/irccd.conf.5 > $(DESTDIR)$(MANDIR)/man5/irccd.conf.5
	$(SUBST) < man/irccdctl.1 > $(DESTDIR)$(MANDIR)/man1/irccdctl.1
