/*
 * uri.cpp -- simple uriparser based parser
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

#include <regex>

#include <uriparser/Uri.h>

#include "scope_exit.hpp"
#include "uri.hpp"

using std::nullopt;
using std::optional;
using std::regex;
using std::regex_match;
using std::smatch;
using std::string;

namespace irccd {

auto uri::parse(const string& link) noexcept -> optional<uri>
{
	/*
	 * The message may contain additional text, example:
	 *
	 * markand: http://example.org check this site
	 */
	regex regex("^(https?:\\/\\/[^\\s]+).*$");
	smatch match;

	if (!regex_match(link, match, regex))
		return nullopt;

	UriParserStateA state;
	UriUriA hnd;
	uri ret;
	scope_exit exit([&hnd] { uriFreeUriMembersA(&hnd); });

	state.uri = &hnd;

	if (uriParseUriA(&state, match[1].str().c_str()) != URI_SUCCESS)
		return nullopt;

	if (hnd.scheme.first)
		ret.scheme = string(hnd.scheme.first, hnd.scheme.afterLast - hnd.scheme.first);

	// We're only interested in http and https.
	if (ret.scheme != "http" && ret.scheme != "https")
		return nullopt;

	// Correct port if not specified.
	if (ret.port.empty())
		ret.port = ret.scheme == "http" ? "80" : "443";

	if (hnd.hostText.first)
		ret.host = string(hnd.hostText.first, hnd.hostText.afterLast - hnd.hostText.first);
	if (hnd.portText.first)
		ret.port = string(hnd.portText.first, hnd.portText.afterLast - hnd.portText.first);

	for (auto p = hnd.pathHead; p != nullptr; p = p->next) {
		ret.path += "/";
		ret.path += string(p->text.first, p->text.afterLast - p->text.first);
	}

	// Correct path if empty.
	if (ret.path.empty())
		ret.path = "/";

	return ret;
}

} // !irccd
