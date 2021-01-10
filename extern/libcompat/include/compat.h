#ifndef LIBCOMPAT_COMPAT_H
#define LIBCOMPAT_COMPAT_H

#include "../src/basename.h"
#include "../src/dirname.h"
#include "../src/err.h"
#include "../src/errc.h"
#include "../src/errx.h"
#include "../src/getopt.h"
#include "../src/pledge.h"
#include "../src/reallocarray.h"
#include "../src/recallocarray.h"
#include "../src/strdup.h"
#include "../src/strlcat.h"
#include "../src/strlcpy.h"
#include "../src/strndup.h"
#include "../src/strnlen.h"
#include "../src/strsep.h"
#include "../src/verr.h"
#include "../src/verrc.h"
#include "../src/verrx.h"
#include "../src/vwarn.h"
#include "../src/vwarnc.h"
#include "../src/vwarnx.h"
#include "../src/warn.h"
#include "../src/warnc.h"
#include "../src/warnx.h"

#include <stdarg.h>
#include <stddef.h>

#ifndef COMPAT_HAVE_BASENAME
char *
basename(char *);
#endif

#ifndef COMPAT_HAVE_DIRNAME
char *
dirname(char *);
#endif

#ifndef COMPAT_HAVE_ERR
void
err(int, const char *, ...);
#endif

#ifndef COMPAT_HAVE_ERRC
void
errc(int, int, const char *, ...);
#endif

#ifndef COMPAT_HAVE_ERRX
void
errx(int, const char *, ...);
#endif

#ifndef COMPAT_HAVE_VERR
void
verr(int, const char *, va_list);
#endif

#ifndef COMPAT_HAVE_VERRC
void
verrc(int, int, const char *, va_list);
#endif

#ifndef COMPAT_HAVE_VERRX
void
verrx(int, const char *, va_list);
#endif

#ifndef COMPAT_HAVE_VWARN
void
vwarn(const char *, va_list);
#endif

#ifndef COMPAT_HAVE_VWARNC
void
vwarnc(int, const char *, va_list);
#endif

#ifndef COMPAT_HAVE_VWARNX
void
vwarnx(const char *, va_list);
#endif

#ifndef COMPAT_HAVE_WARN
void
warn(const char *, ...);
#endif

#ifndef COMPAT_HAVE_WARNC
void
warnc(int, const char *, ...);
#endif

#ifndef COMPAT_HAVE_WARNX
void
warnx(const char *, ...);
#endif

#ifndef COMPAT_HAVE_GETOPT
extern int opterr;
extern int optind;
extern int optopt;
extern int optreset;
extern char *optarg;

int
getopt(int, char **, const char *);
#endif

#ifndef COMPAT_HAVE_PLEDGE
int
pledge(const char *, const char *);
#endif

#ifndef COMPAT_HAVE_REALLOCARRAY
void *
reallocarray(void *, size_t, size_t);
#endif

#ifndef COMPAT_HAVE_RECALLOCARRAY
void *
recallocarray(void *, size_t, size_t, size_t);
#endif

#ifndef COMPAT_HAVE_STRDUP
char *
strdup(const char *);
#endif

#ifndef COMPAT_HAVE_STRLCAT
size_t
strlcat(char *, const char *, size_t);
#endif

#ifndef COMPAT_HAVE_STRLCPY
size_t
strlcpy(char *, const char *, size_t);
#endif

#ifndef COMPAT_HAVE_STRNDUP
char *
strndup(const char *, size_t);
#endif

#ifndef COMPAT_HAVE_STRNLEN
size_t
strnlen(const char *, size_t);
#endif

#ifndef COMPAT_HAVE_STRSEP
char *
strsep(char **, const char *);
#endif

#endif /* !LIBCOMPAT_COMPAT_H */
