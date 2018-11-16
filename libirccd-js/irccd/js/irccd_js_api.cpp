/*
 * irccd_js_api.cpp -- Irccd API
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include "irccd_js_api.hpp"
#include "js_plugin.hpp"

using irccd::daemon::bot;

namespace irccd::js {

namespace {

// {{{ do_raise

template <typename Error>
void do_raise(duk_context* ctx, const Error& ex)
{
	duk::stack_guard sa(ctx, 1);

	duk_get_global_string(ctx, "Irccd");
	duk_get_prop_string(ctx, -1, "SystemError");
	duk_remove(ctx, -2);
	duk::push(ctx, ex.code().value());
	duk::push(ctx, ex.code().message());
	duk_new(ctx, 2);

	(void)duk_throw(ctx);
}

// }}}

// {{{ Irccd.SystemError [constructor]

auto constructor(duk_context* ctx) -> duk_ret_t
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

// }}}

// {{{ definitions

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

// }}}

} // !namespace

void duk::type_traits<std::system_error>::raise(duk_context* ctx, const std::system_error& ex)
{
	do_raise(ctx, ex);
}

void duk::type_traits<boost::system::system_error>::raise(duk_context* ctx, const boost::system::system_error& ex)
{
	do_raise(ctx, ex);
}

auto irccd_js_api::get_name() const noexcept -> std::string_view
{
	return "Irccd";
}

void irccd_js_api::load(bot& bot, std::shared_ptr<js_plugin> plugin)
{
	duk::stack_guard sa(plugin->get_context());

	// irccd.
	duk_push_object(plugin->get_context());

	// Version.
	duk_push_object(plugin->get_context());
	duk::push(plugin->get_context(), IRCCD_VERSION_MAJOR);
	duk_put_prop_string(plugin->get_context(), -2, "major");
	duk::push(plugin->get_context(), IRCCD_VERSION_MINOR);
	duk_put_prop_string(plugin->get_context(), -2, "minor");
	duk::push(plugin->get_context(), IRCCD_VERSION_PATCH);
	duk_put_prop_string(plugin->get_context(), -2, "patch");
	duk_put_prop_string(plugin->get_context(), -2, "version");

	// Create the system_error that inherits from Error.
	duk_push_c_function(plugin->get_context(), constructor, 2);

	// Put errno codes into the irccd.system_error object.
	for (const auto& [k, v] : errors) {
		duk_push_int(plugin->get_context(), v);
		duk_put_prop_string(plugin->get_context(), -2, k.c_str());
	}

	duk_push_object(plugin->get_context());
	duk_get_global_string(plugin->get_context(), "Error");
	duk_get_prop_string(plugin->get_context(), -1, "prototype");
	duk_remove(plugin->get_context(), -2);
	duk_set_prototype(plugin->get_context(), -2);
	duk_put_prop_string(plugin->get_context(), -2, "prototype");
	duk_put_prop_string(plugin->get_context(), -2, "SystemError");

	// Set irccd as global.
	duk_put_global_string(plugin->get_context(), "Irccd");

	// Store global instance.
	duk_push_pointer(plugin->get_context(), &bot);
	duk_put_global_string(plugin->get_context(), DUK_HIDDEN_SYMBOL("irccd-ref"));
}

auto duk::type_traits<bot>::self(duk_context *ctx) -> bot&
{
	duk::stack_guard sa(ctx);

	duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("irccd-ref"));
	const auto ptr = static_cast<bot*>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);

	return *ptr;
}

} // !irccd::js
