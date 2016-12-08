/*
 * mod-irccd.cpp -- Irccd API
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#include <cerrno>
#include <string>
#include <unordered_map>

#include "mod-irccd.hpp"
#include "plugin-js.hpp"
#include "sysconfig.hpp"

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
    { "EBADMSG",            EBADMSG         },
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
    { "EIDRM",              EIDRM           },
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
    { "ENODATA",            ENODATA         },
    { "ENODEV",             ENODEV          },
    { "ENOENT",             ENOENT          },
    { "ENOEXEC",            ENOEXEC         },
    { "ENOLCK",             ENOLCK          },
    { "ENOLINK",            ENOLINK         },
    { "ENOMEM",             ENOMEM          },
    { "ENOMSG",             ENOMSG          },
    { "ENOPROTOOPT",        ENOPROTOOPT     },
    { "ENOSPC",             ENOSPC          },
    { "ENOSR",              ENOSR           },
    { "ENOSTR",             ENOSTR          },
    { "ENOSYS",             ENOSYS          },
    { "ENOTCONN",           ENOTCONN        },
    { "ENOTDIR",            ENOTDIR         },
    { "ENOTEMPTY",          ENOTEMPTY       },
    { "ENOTRECOVERABLE",    ENOTRECOVERABLE },
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
    { "ETIME",              ETIME           },
    { "ETIMEDOUT",          ETIMEDOUT       },
    { "ETXTBSY",            ETXTBSY         },
    { "EWOULDBLOCK",        EWOULDBLOCK     },
    { "EXDEV",              EXDEV           }
};

duk_ret_t constructor(duk_context *ctx)
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

SystemError::SystemError()
    : m_errno(errno)
    , m_message(std::strerror(m_errno))
{
}

SystemError::SystemError(int e, std::string message)
    : m_errno(e)
    , m_message(std::move(message))
{
}

void SystemError::raise(duk_context *ctx) const
{
    StackAssert sa(ctx, 0);

    duk_get_global_string(ctx, "Irccd");
    duk_get_prop_string(ctx, -1, "SystemError");
    duk_remove(ctx, -2);
    duk_push_int(ctx, m_errno);
    dukx_push_std_string(ctx, m_message);
    duk_new(ctx, 2);
    duk_throw(ctx);
}

IrccdModule::IrccdModule() noexcept
    : Module("Irccd")
{
}

void IrccdModule::load(Irccd &irccd, std::shared_ptr<JsPlugin> plugin)
{
    StackAssert sa(plugin->context());

    // Irccd.
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

    // Create the SystemError that inherits from Error.
    duk_push_c_function(plugin->context(), constructor, 2);

    // Put errno codes into the Irccd.SystemError object.
    for (const auto &pair : errors) {
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

    // Set Irccd as global.
    duk_put_global_string(plugin->context(), "Irccd");

    // Store global instance.
    duk_push_pointer(plugin->context(), &irccd);
    duk_put_global_string(plugin->context(), "\xff""\xff""irccd-ref");
}

Irccd &dukx_get_irccd(duk_context *ctx)
{
    StackAssert sa(ctx);

    duk_get_global_string(ctx, "\xff""\xff""irccd-ref");
    Irccd *irccd = static_cast<Irccd *>(duk_to_pointer(ctx, -1));
    duk_pop(ctx);

    return *irccd;
}

} // !irccd
