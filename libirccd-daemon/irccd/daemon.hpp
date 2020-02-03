/*
 * daemon.hpp -- libirccd-daemon convenience header
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_HPP
#define IRCCD_DAEMON_HPP

/**
 * \file daemon.hpp
 * \brief libirccd-daemon convenience header.
 */

#include "sysconfig.hpp"

#include "daemon/bot.hpp"
#include "daemon/dynlib_plugin.hpp"
#include "daemon/hook.hpp"
#include "daemon/hook_service.hpp"
#include "daemon/irc.hpp"
#include "daemon/logger.hpp"
#include "daemon/plugin.hpp"
#include "daemon/plugin_service.hpp"
#include "daemon/rule.hpp"
#include "daemon/rule_service.hpp"
#include "daemon/rule_util.hpp"
#include "daemon/server.hpp"
#include "daemon/server_service.hpp"
#include "daemon/server_util.hpp"
#include "daemon/transport_client.hpp"
#include "daemon/transport_command.hpp"
#include "daemon/transport_server.hpp"
#include "daemon/transport_service.hpp"
#include "daemon/transport_util.hpp"

#endif // !IRCCD_DAEMON_HPP
