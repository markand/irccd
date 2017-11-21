/*
 * network_errc.hpp -- describe some error codes
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

#ifndef IRCCD_COMMON_NETWORK_ERRC_HPP
#define IRCCD_COMMON_NETWORK_ERRC_HPP

/**
 * \file network_errc.hpp
 * \brief Describe some error codes.
 */

#include <boost/system/error_code.hpp>

#include <type_traits>

namespace irccd {

/**
 * \brief Error code for transport/irccdctl
 */
enum class network_errc {
    no_error = 0,           //!< no error (default)
    invalid_program,        //!< connected daemon is not irccd
    invalid_version,        //!< irccd daemon is incompatible
    invalid_auth,           //!< invalid credentials in auth command
    invalid_message,        //!< the message was not JSON
    corrupt_message,        //!< error occured while sending a message
};

/**
 * Get the network category singleton.
 *
 * \return the category for network_errc enum
 */
const boost::system::error_category& network_category() noexcept;

/**
 * Construct an error_code from network_errc enum.
 *
 * \return the error code
 */
boost::system::error_code make_error_code(network_errc errc) noexcept;

} // !irccd

namespace boost {

namespace system {

template <>
struct is_error_code_enum<irccd::network_errc> : public std::true_type {
};

} // !system

} // !boost

#endif // !IRCCD_COMMON_NETWORK_ERRC_HPP
