#
# CMakeLists.txt -- CMake build system for libcompat
#
# Copyright (c) 2020 David Demelier <markand@malikania.fr>
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

cmake_minimum_required(VERSION 3.10)
project(libirccd-compat)

option(COMPAT_WITH_TESTS "Enable tests" Off)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckStructHasMember)
include(CheckSymbolExists)

set(
	SOURCES
	${libirccd-compat_SOURCE_DIR}/src/compat.c
	${libirccd-compat_SOURCE_DIR}/src/compat.h.in
)

set(
	FUNCTIONS
	# BSD extensions.
	err
	errc
	errx
	getprogname
	pledge
	reallocarray
	recallocarray
	setprogname
	strlcat
	strlcpy
	strsep
	strtonum
	verr
	verrc
	verrx
	vsyslog
	vwarn
	vwarnc
	vwarnx
	warn
	warnc
	warnx
	# POSIX functions.
	basename
	dirname
	getopt
	strdup
	strndup
	strnlen
	strtok_r
)

set(
	INCLUDES
	# BSD extensions.
	err.h
	# POSIX extensions.
	unistd.h
	libgen.h
)

foreach (f ${FUNCTIONS})
	string(TOUPPER ${f} var)

	check_function_exists(${f} COMPAT_HAVE_${var})

	if (NOT COMPAT_HAVE_${var})
		list(APPEND SOURCES ${libirccd-compat_SOURCE_DIR}/src/${f}.c)
	endif ()
endforeach ()

# Functionalities we can't replace
check_include_file(syslog.h COMPAT_HAVE_SYSLOG_H)

add_library(libirccd-compat STATIC ${SOURCES})
target_include_directories(
	libirccd-compat
	PUBLIC
		$<BUILD_INTERFACE:${libirccd-compat_BINARY_DIR}>
		$<BUILD_INTERFACE:${libirccd-compat_BINARY_DIR}/irccd>
)
set_target_properties(libirccd-compat PROPERTIES PREFIX "" FOLDER extern)

# Remove useless C4996 warning about "deprecated" functions from MSVC.
if (CMAKE_C_COMPILER_ID MATCHES MSVC)
	target_compile_options(libirccd-compat PUBLIC /wd4996)
endif ()

foreach (i ${INCLUDES})
	string(TOUPPER ${i} var)
	string(REGEX REPLACE "\\." "_" var ${var})

	check_include_file(${i} COMPAT_HAVE_${var})

	if (NOT COMPAT_HAVE_${var})
		file(WRITE ${libirccd-compat_BINARY_DIR}/${i} "/* Empty stub for ${i}. */\n")

		install(
			FILES ${libirccd-compat_BINARY_DIR}/${i}
			DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd/extern
		)
	endif ()
endforeach ()

# POSIX dirent.h
check_include_file(dirent.h COMPAT_HAVE_DIRENT_H)

if (NOT COMPAT_HAVE_DIRENT_H)
	if (CMAKE_SYSTEM_NAME MATCHES "Windows")
		target_include_directories(
			libirccd-compat
			PUBLIC
				$<BUILD_INTERFACE:${libirccd-compat_SOURCE_DIR}/win/dirent>
		)
	endif ()
endif ()

# Specific BSD sys/queue.h file
check_include_file(sys/queue.h COMPAT_HAVE_SYS_QUEUE_H)

if (NOT COMPAT_HAVE_SYS_QUEUE_H)
	target_include_directories(
		libirccd-compat
		PUBLIC
			$<BUILD_INTERFACE:${libirccd-compat_SOURCE_DIR}/extern/queue>
	)

	install(
		FILES ${libirccd-compat_SOURCE_DIR}/extern/queue/sys/queue.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd/extern/sys
	)
else ()
	#
	# Make sure it has at least _SAFE macros because some implementations
	# may provide terribly outdated queue.h (e.g. glibc).
	#
	check_symbol_exists(LIST_FOREACH_SAFE sys/queue.h COMPAT_HAVE_LIST_FOREACH_SAFE)
	check_symbol_exists(SLIST_FOREACH_SAFE sys/queue.h COMPAT_HAVE_SLIST_FOREACH_SAFE)
	check_symbol_exists(STAILQ_FOREACH_SAFE sys/queue.h COMPAT_HAVE_STAILQ_FOREACH_SAFE)
	check_symbol_exists(TAILQ_FOREACH_SAFE sys/queue.h COMPAT_HAVE_TAILQ_FOREACH_SAFE)

	if (NOT COMPAT_HAVE_LIST_FOREACH_SAFE OR
	    NOT COMPAT_HAVE_SLIST_FOREACH_SAFE OR
	    NOT COMPAT_HAVE_STAILQ_FOREACH_SAFE OR
	    NOT COMPAT_HAVE_TAILQ_FOREACH_SAFE)
		target_include_directories(
			libirccd-compat
			PUBLIC
				$<BUILD_INTERFACE:${libirccd-compat_SOURCE_DIR}/extern/queue>
		)
	endif ()
endif ()

# Math library
find_library(COMPAT_HAVE_LIBM m)

if (NOT COMPAT_HAVE_LIBM)
	add_library(m INTERFACE)
endif ()

# POSIX dlfcn.h
check_include_file(dlfcn.h COMPAT_HAVE_DLFCN_H)

if (NOT COMPAT_HAVE_DLFCN_H)
	if (CMAKE_SYSTEM_NAME MATCHES "Windows")
		add_subdirectory(win/dlfcn)
	endif ()
else ()
	# Some systems may not provide the dl library.
	find_library(COMPAT_HAVE_LIBDL dl)

	if (NOT COMPAT_HAVE_LIBDL)
		add_library(dl INTERFACE)
	endif ()
endif ()

# struct stat fields
check_struct_has_member("struct stat" st_atime sys/stat.h COMPAT_HAVE_STAT_ST_ATIME)
check_struct_has_member("struct stat" st_blksize sys/stat.h COMPAT_HAVE_STAT_ST_BLKSIZE)
check_struct_has_member("struct stat" st_blocks sys/stat.h COMPAT_HAVE_STAT_ST_BLOCKS)
check_struct_has_member("struct stat" st_ctime sys/stat.h COMPAT_HAVE_STAT_ST_CTIME)
check_struct_has_member("struct stat" st_dev sys/stat.h COMPAT_HAVE_STAT_ST_DEV)
check_struct_has_member("struct stat" st_gid sys/stat.h COMPAT_HAVE_STAT_ST_GID)
check_struct_has_member("struct stat" st_ino sys/stat.h COMPAT_HAVE_STAT_ST_INO)
check_struct_has_member("struct stat" st_mode sys/stat.h COMPAT_HAVE_STAT_ST_MODE)
check_struct_has_member("struct stat" st_mtime sys/stat.h COMPAT_HAVE_STAT_ST_MTIME)
check_struct_has_member("struct stat" st_nlink sys/stat.h COMPAT_HAVE_STAT_ST_NLINK)
check_struct_has_member("struct stat" st_rdev sys/stat.h COMPAT_HAVE_STAT_ST_RDEV)
check_struct_has_member("struct stat" st_size sys/stat.h COMPAT_HAVE_STAT_ST_SIZE)
check_struct_has_member("struct stat" st_uid sys/stat.h COMPAT_HAVE_STAT_ST_UID)

if (COMPAT_WITH_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif ()

configure_file(
	${libirccd-compat_SOURCE_DIR}/src/compat.h.in
	${libirccd-compat_BINARY_DIR}/irccd/compat.h
)

install(
	FILES ${libirccd-compat_BINARY_DIR}/irccd/compat.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd
)
