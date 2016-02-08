#
# IrccdDefinePlugin.cmake -- CMake build system for irccd
#
# Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

#
# irccd_define_plugin
# -------------------------------------------------------------------
#
# Define a JavaScript plugin, the file will be configured using configure_file and then installed.
#
# Parameters:
#	file		- The plugin full file path
#
macro(irccd_define_plugin file)
	get_filename_component(name ${file} NAME)

	if (IRCCD_RELOCATABLE)
		set(base ${CMAKE_BINARY_DIR}/fakeroot/${WITH_PLUGINDIR})
	else ()
		set(base ${CMAKE_BINARY_DIR}/plugin)
	endif ()

	# Substitude variables in the JavaScript file.
	configure_file(${file} ${base}/${name})

	# Install the plugin file.
	install(FILES ${base}/${name} DESTINATION ${WITH_PLUGINDIR})
endmacro()
