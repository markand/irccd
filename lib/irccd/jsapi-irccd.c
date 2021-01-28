/*
 * jsapi-irccd.c -- Irccd API
 *
 * Copyright (c) 2013-2021 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <string.h>

#include <duktape.h>

#include "config.h"
#include "util.h"

static int
SystemError_constructor(duk_context *ctx)
{
	duk_push_this(ctx);
	duk_push_int(ctx, duk_require_int(ctx, 0));
	duk_put_prop_string(ctx, -2, "errno");
	duk_push_string(ctx, duk_require_string(ctx, 1));
	duk_put_prop_string(ctx, -2, "message");
	duk_push_string(ctx, "SystemError");
	duk_put_prop_string(ctx, -2, "name");
	duk_pop(ctx);

	return 0;
}

/* {{{ Constants for errno. */

static const struct {
	const char *name;
	int value;
} errors[] = {
#if defined(E2BIG)
	{ "E2BIG",              E2BIG           },
#endif
#if defined(EACCES)
	{ "EACCES",             EACCES          },
#endif
#if defined(EADDRINUSE)
	{ "EADDRINUSE",         EADDRINUSE      },
#endif
#if defined(EADDRNOTAVAIL)
	{ "EADDRNOTAVAIL",      EADDRNOTAVAIL   },
#endif
#if defined(EAFNOSUPPORT)
	{ "EAFNOSUPPORT",       EAFNOSUPPORT    },
#endif
#if defined(EAGAIN)
	{ "EAGAIN",             EAGAIN          },
#endif
#if defined(EALREADY)
	{ "EALREADY",           EALREADY        },
#endif
#if defined(EBADF)
	{ "EBADF",              EBADF           },
#endif
#if defined(EBADMSG)
	{ "EBADMSG",            EBADMSG         },
#endif
#if defined(EBUSY)
	{ "EBUSY",              EBUSY           },
#endif
#if defined(ECANCELED)
	{ "ECANCELED",          ECANCELED       },
#endif
#if defined(ECHILD)
	{ "ECHILD",             ECHILD          },
#endif
#if defined(ECONNABORTED)
	{ "ECONNABORTED",       ECONNABORTED    },
#endif
#if defined(ECONNREFUSED)
	{ "ECONNREFUSED",       ECONNREFUSED    },
#endif
#if defined(ECONNREFUSED)
	{ "ECONNRESET",         ECONNRESET      },
#endif
#if defined(EDEADLK)
	{ "EDEADLK",            EDEADLK         },
#endif
#if defined(EDESTADDRREQ)
	{ "EDESTADDRREQ",       EDESTADDRREQ    },
#endif
#if defined(EDOM)
	{ "EDOM",               EDOM            },
#endif
#if defined(EEXIST)
	{ "EEXIST",             EEXIST          },
#endif
#if defined(EFAULT)
	{ "EFAULT",             EFAULT          },
#endif
#if defined(EFBIG)
	{ "EFBIG",              EFBIG           },
#endif
#if defined(EHOSTUNREACH)
	{ "EHOSTUNREACH",       EHOSTUNREACH    },
#endif
#if defined(EIDRM)
	{ "EIDRM",              EIDRM           },
#endif
#if defined(EILSEQ)
	{ "EILSEQ",             EILSEQ          },
#endif
#if defined(EINPROGRESS)
	{ "EINPROGRESS",        EINPROGRESS     },
#endif
#if defined(EINTR)
	{ "EINTR",              EINTR           },
#endif
#if defined(EINVAL)
	{ "EINVAL",             EINVAL          },
#endif
#if defined(EIO)
	{ "EIO",                EIO             },
#endif
#if defined(EISCONN)
	{ "EISCONN",            EISCONN         },
#endif
#if defined(EISDIR)
	{ "EISDIR",             EISDIR          },
#endif
#if defined(ELOOP)
	{ "ELOOP",              ELOOP           },
#endif
#if defined(EMFILE)
	{ "EMFILE",             EMFILE          },
#endif
#if defined(EMLINK)
	{ "EMLINK",             EMLINK          },
#endif
#if defined(EMSGSIZE)
	{ "EMSGSIZE",           EMSGSIZE        },
#endif
#if defined(ENAMETOOLONG)
	{ "ENAMETOOLONG",       ENAMETOOLONG    },
#endif
#if defined(ENETDOWN)
	{ "ENETDOWN",           ENETDOWN        },
#endif
#if defined(ENETRESET)
	{ "ENETRESET",          ENETRESET       },
#endif
#if defined(ENETUNREACH)
	{ "ENETUNREACH",        ENETUNREACH     },
#endif
#if defined(ENFILE)
	{ "ENFILE",             ENFILE          },
#endif
#if defined(ENOBUFS)
	{ "ENOBUFS",            ENOBUFS         },
#endif
#if defined(ENODATA)
	{ "ENODATA",            ENODATA         },
#endif
#if defined(ENODEV)
	{ "ENODEV",             ENODEV          },
#endif
#if defined(ENOENT)
	{ "ENOENT",             ENOENT          },
#endif
#if defined(ENOEXEC)
	{ "ENOEXEC",            ENOEXEC         },
#endif
#if defined(ENOLCK)
	{ "ENOLCK",             ENOLCK          },
#endif
#if defined(ENOLINK)
	{ "ENOLINK",            ENOLINK         },
#endif
#if defined(ENOMEM)
	{ "ENOMEM",             ENOMEM          },
#endif
#if defined(ENOMSG)
	{ "ENOMSG",             ENOMSG          },
#endif
#if defined(ENOPROTOOPT)
	{ "ENOPROTOOPT",        ENOPROTOOPT     },
#endif
#if defined(ENOSPC)
	{ "ENOSPC",             ENOSPC          },
#endif
#if defined(ENOSR)
	{ "ENOSR",              ENOSR           },
#endif
#if defined(ENOSTR)
	{ "ENOSTR",             ENOSTR          },
#endif
#if defined(ENOSYS)
	{ "ENOSYS",             ENOSYS          },
#endif
#if defined(ENOTCONN)
	{ "ENOTCONN",           ENOTCONN        },
#endif
#if defined(ENOTDIR)
	{ "ENOTDIR",            ENOTDIR         },
#endif
#if defined(ENOTEMPTY)
	{ "ENOTEMPTY",          ENOTEMPTY       },
#endif
#if defined(ENOTRECOVERABLE)
	{ "ENOTRECOVERABLE",    ENOTRECOVERABLE },
#endif
#if defined(ENOTSOCK)
	{ "ENOTSOCK",           ENOTSOCK        },
#endif
#if defined(ENOTSUP)
	{ "ENOTSUP",            ENOTSUP         },
#endif
#if defined(ENOTTY)
	{ "ENOTTY",             ENOTTY          },
#endif
#if defined(ENXIO)
	{ "ENXIO",              ENXIO           },
#endif
#if defined(EOPNOTSUPP)
	{ "EOPNOTSUPP",         EOPNOTSUPP      },
#endif
#if defined(EOVERFLOW)
	{ "EOVERFLOW",          EOVERFLOW       },
#endif
#if defined(EOWNERDEAD)
	{ "EOWNERDEAD",         EOWNERDEAD      },
#endif
#if defined(EPERM)
	{ "EPERM",              EPERM           },
#endif
#if defined(EPIPE)
	{ "EPIPE",              EPIPE           },
#endif
#if defined(EPROTO)
	{ "EPROTO",             EPROTO          },
#endif
#if defined(EPROTONOSUPPORT)
	{ "EPROTONOSUPPORT",    EPROTONOSUPPORT },
#endif
#if defined(EPROTOTYPE)
	{ "EPROTOTYPE",         EPROTOTYPE      },
#endif
#if defined(ERANGE)
	{ "ERANGE",             ERANGE          },
#endif
#if defined(EROFS)
	{ "EROFS",              EROFS           },
#endif
#if defined(ESPIPE)
	{ "ESPIPE",             ESPIPE          },
#endif
#if defined(ESRCH)
	{ "ESRCH",              ESRCH           },
#endif
#if defined(ETIME)
	{ "ETIME",              ETIME           },
#endif
#if defined(ETIMEDOUT)
	{ "ETIMEDOUT",          ETIMEDOUT       },
#endif
#if defined(ETXTBSY)
	{ "ETXTBSY",            ETXTBSY         },
#endif
#if defined(EWOULDBLOCK)
	{ "EWOULDBLOCK",        EWOULDBLOCK     },
#endif
#if defined(EXDEV)
	{ "EXDEV",              EXDEV           }
#endif
};

/* }}} */

static int
print(duk_context *ctx)
{
	puts(duk_require_string(ctx, 0));

	return 0;
}

void
irc_jsapi_load(duk_context *ctx)
{
	/* Irccd (global object) */
	duk_push_object(ctx);

	/* Irccd.version (property) */
	duk_push_object(ctx);
	duk_push_int(ctx, IRCCD_VERSION_MAJOR);
	duk_put_prop_string(ctx, -2, "major");
	duk_push_int(ctx, IRCCD_VERSION_MINOR);
	duk_put_prop_string(ctx, -2, "minor");
	duk_push_int(ctx, IRCCD_VERSION_PATCH);
	duk_put_prop_string(ctx, -2, "patch");
	duk_put_prop_string(ctx, -2, "version");

	/* Create the system_error that inherits from Error. */
	duk_push_c_function(ctx, SystemError_constructor, 2);

	/* Put errno codes into the Irccd.SystemError object. */
	for (size_t i = 0; i < IRC_UTIL_SIZE(errors); ++i) {
		duk_push_int(ctx, errors[i].value);
		duk_put_prop_string(ctx, -2, errors[i].name);
	}

	duk_push_object(ctx);
	duk_get_global_string(ctx, "Error");
	duk_get_prop_string(ctx, -1, "prototype");
	duk_remove(ctx, -2);
	duk_set_prototype(ctx, -2);
	duk_put_prop_string(ctx, -2, "prototype");
	duk_put_prop_string(ctx, -2, "SystemError");

	/* Set Irccd as global. */
	duk_put_global_string(ctx, "Irccd");

	/* Convenient global "print" function. */
	duk_push_c_function(ctx, print, 1);
	duk_put_global_string(ctx, "print");
}
