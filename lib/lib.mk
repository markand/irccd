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
		-e "s,@MAJOR@,$(MAJOR),g" \
		-e "s,@MINOR@,$(MINOR),g" \
		-e "s,@PATCH@,$(PATCH),g" \
		-e "s,@VARDIR@,$(VARDIR),g" \
		-e "s,@ETCDIR@,$(ETCDIR),g" \
		-e "s,@LIBDIR@,$(LIBDIR),g" \
		-e "s,@SHAREDIR@,$(SHAREDIR),g" \
		-e "s,@JS@,$(JS),g" \
		-e "s,@SSL@,$(SSL),g" \
		-e "s,@HTTP@,$(HTTP),g" \
		< $< > $@

$(LIBIRCCD): $(LIBIRCCD_OBJS) | $(LIBEV)
$(LIBIRCCD_SRCS): lib/irccd/config.h
$(LIBIRCCD_OBJS): private CFLAGS += $(LIBBSD_CFLAGS) $(LIBEV_CFLAGS) $(LIBUTLIST_CFLAGS)

all:: $(LIBIRCCD)

clean::
	rm -f $(LIBIRCCD) $(LIBIRCCD_DEPS) $(LIBIRCCD_OBJS) lib/irccd/config.h

-include $(LIBIRCCD_DEPS)
