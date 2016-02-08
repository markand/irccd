/*
 * filesystem.h -- some file system operation
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

#ifndef _IRCCD_FILESYSTEM_H_
#define _IRCCD_FILESYSTEM_H_

#include <string>

namespace irccd {

namespace fs {

extern const char Separator;

std::string baseName(std::string path);
std::string dirName(std::string path);
bool isAbsolute(const std::string &path) noexcept;
bool isRelative(const std::string &path) noexcept;
bool exists(const std::string &path);
void mkdir(const std::string &dir, int mode = 0700);
std::string cwd();

} // !fs

} // !irccd

#endif // !_IRCCD_FILESYSTEM_H_
