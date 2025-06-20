install::
	mkdir -p $(DESTDIR)$(LIBDIR)/systemd/system
	sed \
		-e "s,@UID@,$(UID),g" \
		-e "s,@GID@,$(GID),g" \
		-e "s,@PATH@,$(BINDIR)/irccd",g \
		< systemd/irccd.service.in \
		> $(DESTDIR)$(LIBDIR)/systemd/system/irccd.service
