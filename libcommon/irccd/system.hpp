/*
 * system.hpp -- platform dependent functions for system inspection
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

#ifndef IRCCD_SYSTEM_HPP
#define IRCCD_SYSTEM_HPP

/**
 * \file system.hpp
 * \brief System dependant functions
 */

#include <cstdint>
#include <string>

#include "sysconfig.hpp"

namespace irccd {

/**
 * \brief Namespace for system functions.
 */
namespace sys {

/**
 * Set the program name, needed for some functions or some systems.
 *
 * \param name the program name
 */
IRCCD_EXPORT void setProgramName(std::string name) noexcept;

/**
 * Get the program name.
 *
 * \return the program name
 */
IRCCD_EXPORT const std::string &programName() noexcept;

/**
 * Get the system name.
 *
 * \return the name
 */
IRCCD_EXPORT std::string name();

/**
 * Get the system version.
 *
 * \return the version
 */
IRCCD_EXPORT std::string version();

/**
 * Get the number of seconds elapsed since the boottime.
 *
 * \return the number of seconds
 */
IRCCD_EXPORT uint64_t uptime();

/**
 * Get the milliseconds elapsed since the application
 * startup.
 *
 * \return the milliseconds
 */
IRCCD_EXPORT uint64_t ticks();

/**
 * Get an environment variable.
 *
 * \return the value or empty string
 */
IRCCD_EXPORT std::string env(const std::string &var);

/**
 * Get home directory usually /home/foo
 *
 * \return the home directory
 */
IRCCD_EXPORT std::string home();

#if defined(HAVE_SETUID)

/**
 * Set the effective uid by name or numeric value.
 *
 * \param value the value
 */
IRCCD_EXPORT void setUid(const std::string &value);

#endif

#if defined(HAVE_SETGID)

/**
 * Set the effective gid by name or numeric value.
 *
 * \param value the value
 */
IRCCD_EXPORT void setGid(const std::string &value);

#endif

} // !sys

} // !irccd

#endif // !IRCCD_SYSTEM_HPP