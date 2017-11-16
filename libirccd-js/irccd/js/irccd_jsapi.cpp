/*
 * irccd_jsapi.cpp -- Irccd API
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include <irccd/sysconfig.hpp>

#include <cerrno>
#include <string>
#include <unordered_map>

#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"

namespace irccd {

namespace {

const std::unordered_map<std::string, int> errors{
    { "E2BIG",              E2BIG           },
    { "EACCES",             EACCES          },
    { "EADDRINUSE",         EADDRINUSE      },
    { "EADDRNOTAVAIL",      EADDRNOTAVAIL   },
    { "EAFNOSUPPORT",       EAFNOSUPPORT    },
    { "EAGAIN",             EAGAIN          },
    { "EALREADY",           EALREADY        },
    { "EBADF",              EBADF           },
#if defined(EBADMSG)
    { "EBADMSG",            EBADMSG         },
#endif
    { "EBUSY",              EBUSY           },
    { "ECANCELED",          ECANCELED       },
    { "ECHILD",             ECHILD          },
    { "ECONNABORTED",       ECONNABORTED    },
    { "ECONNREFUSED",       ECONNREFUSED    },
    { "ECONNRESET",         ECONNRESET      },
    { "EDEADLK",            EDEADLK         },
    { "EDESTADDRREQ",       EDESTADDRREQ    },
    { "EDOM",               EDOM            },
    { "EEXIST",             EEXIST          },
    { "EFAULT",             EFAULT          },
    { "EFBIG",              EFBIG           },
    { "EHOSTUNREACH",       EHOSTUNREACH    },
#if defined(EIDRM)
    { "EIDRM",              EIDRM           },
#endif
    { "EILSEQ",             EILSEQ          },
    { "EINPROGRESS",        EINPROGRESS     },
    { "EINTR",              EINTR           },
    { "EINVAL",             EINVAL          },
    { "EIO",                EIO             },
    { "EISCONN",            EISCONN         },
    { "EISDIR",             EISDIR          },
    { "ELOOP",              ELOOP           },
    { "EMFILE",             EMFILE          },
    { "EMLINK",             EMLINK          },
    { "EMSGSIZE",           EMSGSIZE        },
    { "ENAMETOOLONG",       ENAMETOOLONG    },
    { "ENETDOWN",           ENETDOWN        },
    { "ENETRESET",          ENETRESET       },
    { "ENETUNREACH",        ENETUNREACH     },
    { "ENFILE",             ENFILE          },
    { "ENOBUFS",            ENOBUFS         },
#if defined(ENODATA)
    { "ENODATA",            ENODATA         },
#endif
    { "ENODEV",             ENODEV          },
    { "ENOENT",             ENOENT          },
    { "ENOEXEC",            ENOEXEC         },
    { "ENOLCK",             ENOLCK          },
#if defined(ENOLINK)
    { "ENOLINK",            ENOLINK         },
#endif
    { "ENOMEM",             ENOMEM          },
#if defined(ENOMSG)
    { "ENOMSG",             ENOMSG          },
#endif
    { "ENOPROTOOPT",        ENOPROTOOPT     },
    { "ENOSPC",             ENOSPC          },
#if defined(ENOSR)
    { "ENOSR",              ENOSR           },
#endif
#if defined(ENOSTR)
    { "ENOSTR",             ENOSTR          },
#endif
    { "ENOSYS",             ENOSYS          },
    { "ENOTCONN",           ENOTCONN        },
    { "ENOTDIR",            ENOTDIR         },
    { "ENOTEMPTY",          ENOTEMPTY       },
#if defined(ENOTRECOVERABLE)
    { "ENOTRECOVERABLE",    ENOTRECOVERABLE },
#endif
    { "ENOTSOCK",           ENOTSOCK        },
    { "ENOTSUP",            ENOTSUP         },
    { "ENOTTY",             ENOTTY          },
    { "ENXIO",              ENXIO           },
    { "EOPNOTSUPP",         EOPNOTSUPP      },
    { "EOVERFLOW",          EOVERFLOW       },
    { "EOWNERDEAD",         EOWNERDEAD      },
    { "EPERM",              EPERM           },
    { "EPIPE",              EPIPE           },
    { "EPROTO",             EPROTO          },
    { "EPROTONOSUPPORT",    EPROTONOSUPPORT },
    { "EPROTOTYPE",         EPROTOTYPE      },
    { "ERANGE",             ERANGE          },
    { "EROFS",              EROFS           },
    { "ESPIPE",             ESPIPE          },
    { "ESRCH",              ESRCH           },
#if defined(ETIME)
    { "ETIME",              ETIME           },
#endif
    { "ETIMEDOUT",          ETIMEDOUT       },
#if defined(ETXTBSY)
    { "ETXTBSY",            ETXTBSY         },
#endif
    { "EWOULDBLOCK",        EWOULDBLOCK     },
    { "EXDEV",              EXDEV           }
};

duk_ret_t constructor(duk_context* ctx)
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

} // !namespace

system_error::system_error()
    : errno_(errno)
    , message_(std::strerror(errno_))
{
}

system_error::system_error(int e, std::string message)
    : errno_(e)
    , message_(std::move(message))
{
}

void system_error::raise(duk_context *ctx) const
{
    StackAssert sa(ctx, 0);

    duk_get_global_string(ctx, "Irccd");
    duk_get_prop_string(ctx, -1, "SystemError");
    duk_remove(ctx, -2);
    duk_push_int(ctx, errno_);
    dukx_push_std_string(ctx, message_);
    duk_new(ctx, 2);
    duk_throw(ctx);
}

std::string irccd_jsapi::name() const
{
    return "Irccd";
}

void irccd_jsapi::load(irccd& irccd, std::shared_ptr<js_plugin> plugin)
{
    StackAssert sa(plugin->context());

    // irccd.
    duk_push_object(plugin->context());

    // Version.
    duk_push_object(plugin->context());
    duk_push_int(plugin->context(), IRCCD_VERSION_MAJOR);
    duk_put_prop_string(plugin->context(), -2, "major");
    duk_push_int(plugin->context(), IRCCD_VERSION_MINOR);
    duk_put_prop_string(plugin->context(), -2, "minor");
    duk_push_int(plugin->context(), IRCCD_VERSION_PATCH);
    duk_put_prop_string(plugin->context(), -2, "patch");
    duk_put_prop_string(plugin->context(), -2, "version");

    // Create the system_error that inherits from Error.
    duk_push_c_function(plugin->context(), constructor, 2);

    // Put errno codes into the irccd.system_error object.
    for (const auto& pair : errors) {
        duk_push_int(plugin->context(), pair.second);
        duk_put_prop_string(plugin->context(), -2, pair.first.c_str());
    }

    duk_push_object(plugin->context());
    duk_get_global_string(plugin->context(), "Error");
    duk_get_prop_string(plugin->context(), -1, "prototype");
    duk_remove(plugin->context(), -2);
    duk_set_prototype(plugin->context(), -2);
    duk_put_prop_string(plugin->context(), -2, "prototype");
    duk_put_prop_string(plugin->context(), -2, "SystemError");

    // Set irccd as global.
    duk_put_global_string(plugin->context(), "Irccd");

    // Store global instance.
    duk_push_pointer(plugin->context(), &irccd);
    duk_put_global_string(plugin->context(), "\xff""\xff""irccd-ref");
}

irccd& dukx_get_irccd(duk_context *ctx)
{
    StackAssert sa(ctx);

    duk_get_global_string(ctx, "\xff""\xff""irccd-ref");
    auto ptr = static_cast<irccd*>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return *ptr;
}

} // !irccd
