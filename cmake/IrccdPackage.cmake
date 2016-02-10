#
# CPackConfing.cmake -- CMake build system for irccd
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

set(CPACK_SOURCE_PACKAGE_FILE_NAME "irccd-${IRCCD_VERSION}")
set(CPACK_SOURCE_GENERATOR TXZ ZIP)
set(CPACK_SOURCE_IGNORE_FILES .hg .hgignore)

#
# Define the binary package name.
# -------------------------------------------------------------------
#

if (WIN32)
	set(PKGSYS "Windows")
	set(PKGSUFFIX "exe")
endif ()

if (IRCCD_64BITS)
	set(PKGARCH "amd64")
else ()
	set(PKGARCH "x86")
endif ()

#
# Create the QtIFW hierarchy.
# -------------------------------------------------------------------
#

# Custom package_ifw on Windows
if (IRCCD_PACKAGE)
	set(CONFDIR ${CMAKE_BINARY_DIR}/installer/config)
	set(PKGDIR ${CMAKE_BINARY_DIR}/installer/packages)
	set(PKGNAME "irccd-${IRCCD_VERSION}-${PKGSYS}-${PKGARCH}.${PKGSUFFIX}")

	# Configure some QtIFW files and their meta packages.
	file(COPY cmake/installer/LICENSE DESTINATION ${PKGDIR}/base/meta)

	# QtIFW configuration file
	configure_file(cmake/installer/config/config.xml.in ${CONFDIR}/config.xml)

	# Meta packages
	configure_file(cmake/installer/packages/meta-programs.xml.in ${PKGDIR}/base/meta/package.xml)
	configure_file(cmake/installer/packages/meta-plugins.xml.in ${PKGDIR}/plugins/meta/package.xml)

	# Irccd, irccdctl and docs
	file(
		MAKE_DIRECTORY 
		${PKGDIR}/base.irccd/data/${WITH_BINDIR}
		${PKGDIR}/base.irccdctl/data/${WITH_BINDIR}
		${PKGDIR}/docs/data/${WITH_DOCDIR}
	)

	configure_file(cmake/installer/packages/irccd.xml.in ${PKGDIR}/base.irccd/meta/package.xml)
	configure_file(cmake/installer/packages/irccd.xml.in ${PKGDIR}/base.irccdctl/meta/package.xml)
	configure_file(cmake/installer/packages/docs.xml.in ${PKGDIR}/docs/meta/package.xml)

	# Main dependencies.
	set(dependencies irccd irccdctl docs)

	# Build commands for plugins.
	foreach (plugin ${IRCCD_PLUGINS})
		list(APPEND dependencies plugin-${plugin})
		file(MAKE_DIRECTORY ${PKGDIR}/plugins.${plugin}/data/${WITH_PLUGINDIR})
		set(IRCCD_PLUGIN_NAME ${plugin})
		configure_file(cmake/installer/packages/plugin.xml.in ${PKGDIR}/plugins.${plugin}/meta/package.xml)
		list(
			APPEND
			PLUGIN_COMMANDS
			COMMAND ${CMAKE_COMMAND} -E copy ${IRCCD_FAKEDIR}/${WITH_PLUGINDIR}/${plugin}.js ${PKGDIR}/plugins.${plugin}/data/${WITH_PLUGINDIR}
		)
	endforeach ()

	# Target for building the package.
	add_custom_target(
		package_ifw
		${PLUGIN_COMMANDS}
		COMMAND
			${CMAKE_COMMAND} -E copy $<TARGET_FILE:irccd> ${PKGDIR}/base.irccd/data/${WITH_BINDIR}
		COMMAND
			${CMAKE_COMMAND} -E copy $<TARGET_FILE:irccdctl> ${PKGDIR}/base.irccdctl/data/${WITH_BINDIR}
		COMMAND
			${CMAKE_COMMAND} -E copy_directory ${IRCCD_FAKEDIR}/${WITH_DOCDIR} ${PKGDIR}/docs/data/${WITH_DOCDIR}
		COMMAND
			${QtIFW_CREATOR} -c ${CONFDIR}/config.xml -p ${PKGDIR} ${CMAKE_BINARY_DIR}/${PKGNAME}
		COMMENT "Generating ${CMAKE_BINARY_DIR}/${PKGNAME}"
		DEPENDS ${dependencies}
		VERBATIM
	)
endif ()
