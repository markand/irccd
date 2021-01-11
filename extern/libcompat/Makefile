#
# Makefile -- POSIX make for libcompat
#
# Copyright (c) 2020 David Demelier <markand@malikania.fr>
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

.POSIX:

.SUFFIXES:
.SUFFIXES: .c .o .h .sh

FUNCTIONS_SRCS= src/basename.c \
                src/dirname.c \
                src/err.c \
                src/errc.c \
                src/errx.c \
                src/getopt.c \
                src/pledge.c \
                src/reallocarray.c \
                src/recallocarray.c \
                src/strdup.c \
                src/strlcat.c \
                src/strlcpy.c \
                src/strndup.c \
                src/strnlen.c \
                src/strsep.c \
                src/verr.c \
                src/verrc.c \
                src/verrx.c \
                src/vwarn.c \
                src/vwarnc.c \
                src/vwarnx.c \
                src/warn.c \
                src/warnc.c \
                src/warnx.c

INCLUDES_SRCS=  include/err.h \
                include/libgen.h \
                include/unistd.h

LIBS_SRCS=      lib/libm.a \
                lib/libdl.a \
                lib/libintl.a

FUNCTIONS_OBJS= ${FUNCTIONS_SRCS:.c=.o}
FUNCTIONS_HDRS= ${FUNCTIONS_SRCS:.c=.h}

LIB=            libirccd-compat.a

all: ${LIB}

.sh:
	@cp $< $@
	@chmod +x $@

.c.o:
	@CC="${CC}" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" ./trycompile -c ${*F}

.c.h:
	CC="${CC}" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}" ./trycompile ${*F}

${FUNCTIONS_HDRS} ${FUNCTIONS_OBJS}: trycompile

include: Makefile tryinclude
	@for h in ${INCLUDES_SRCS}; do CC="${CC}" CFLAGS="${CFLAGS}" ./tryinclude `basename $$h`; done
	@touch include

lib: Makefile trylib
	@for l in ${LIBS_SRCS}; do AR="${AR}" CC="${CC}" CFLAGS="${CFLAGS}" ./trylib `basename $$l`; done
	@touch lib

${LIB}: include lib ${FUNCTIONS_HDRS} ${FUNCTIONS_OBJS}
	@mkdir -p ${@D}
	@${AR} -rc $@ ${FUNCTIONS_OBJS} >/dev/null 2>&1

clean:
	@rm -f trycompile tryinclude trylib
	@rm -f ${LIB}
	@rm -f ${FUNCTIONS_OBJS}
	@rm -f ${FUNCTIONS_HDRS}
	@rm -f ${INCLUDES_SRCS}

.PHONY: all clean
