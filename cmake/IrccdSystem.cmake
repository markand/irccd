#
# Config.cmake -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)

#
# Global compile flags.
# -------------------------------------------------------------------
#

if (MINGW)
	set(CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0600 ${CMAKE_CXX_FLAGS}")
endif ()

if (CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(IRCCD_64BITS TRUE)
else ()
	set(IRCCD_64BITS FALSE)
endif ()

#
# Portability requirements.
# -------------------------------------------------------------------
#

check_type_size(int8_t IRCCD_HAVE_INT8)
check_type_size(uint8_t IRCCD_HAVE_UINT8)
check_type_size(int16_t IRCCD_HAVE_INT16)
check_type_size(uint16_t IRCCD_HAVE_UINT16)
check_type_size(int32_t IRCCD_HAVE_INT32)
check_type_size(uint32_t IRCCD_HAVE_UINT32)
check_type_size(int64_t IRCCD_HAVE_INT64)
check_type_size(uint64_t IRCCD_HAVE_UINT64)

#
# Where any of these function / feature is required, include the <irccd/sysconfig.h> file.
#
# The following variables are defined in irccd/sysconfig.h
#
# IRCCD_HAVE_POPEN              True if has popen(3) function (in stdio.h)
# IRCCD_HAVE_SETPROGNAME        True if setprogname(3) is available from C library,
# IRCCD_HAVE_STAT               True if has stat(2) function (and sys/types.h and sys/stat.h),
# IRCCD_HAVE_STAT_ST_DEV        The struct stat has st_dev field,
# IRCCD_HAVE_STAT_ST_INO        The struct stat has st_ino field,
# IRCCD_HAVE_STAT_ST_NLINK      The struct stat has st_nlink field,
# IRCCD_HAVE_STAT_ST_UID        The struct stat has st_uid field,
# IRCCD_HAVE_STAT_ST_GID        The struct stat has st_gid field,
# IRCCD_HAVE_STAT_ST_ATIME      The struct stat has st_atime field,
# IRCCD_HAVE_STAT_ST_MTIME      The struct stat has st_mtime field,
# IRCCD_HAVE_STAT_ST_CTIME      The struct stat has st_ctime field,
# IRCCD_HAVE_STAT_ST_SIZE       The struct stat has st_size field,
# IRCCD_HAVE_STAT_ST_BLKSIZE    The struct stat has st_blksize field,
# IRCCD_HAVE_STAT_ST_BLOCKS     The struct stat has st_blocks field,
# IRCCD_HAVE_SYSLOG             True if syslog functions are available (and syslog.h),
#

# Check for unistd.h
check_include_file(unistd.h IRCCD_HAVE_UNISTD_H)

# Check for sys/types.h
check_include_file(sys/types.h IRCCD_HAVE_SYS_TYPES_H)

# Check of setprogname(3) function, include:
#
# #include <cstdlib>
check_function_exists(setprogname IRCCD_HAVE_SETPROGNAME)

# popen() function
#
# If IRCCD_HAVE_POPEN is defined, include;
#
# #include <cstdio>
check_function_exists(popen IRCCD_HAVE_POPEN)

# stat(2) function
#
# If IRCCD_HAVE_STAT is defined, include:
#
# #include <sys/types.h>
# #include <sys/stat.h>
check_include_file(sys/stat.h IRCCD_HAVE_SYS_STAT_H)
check_function_exists(stat IRCCD_HAVE_STAT)

# If the sys/stat.h is not found, we disable stat(2)
if (NOT IRCCD_HAVE_SYS_STAT_H OR NOT IRCCD_HAVE_SYS_TYPES_H)
	set(IRCCD_HAVE_STAT FALSE)
endif ()

# syslog functions
#
# If IRCCD_HAVE_SYSLOG is defined, include:
#
# #include <syslog.h>
check_include_file(syslog.h IRCCD_HAVE_SYSLOG_H)
check_function_exists(openlog IRCCD_HAVE_OPENLOG)
check_function_exists(syslog IRCCD_HAVE_SYSLOG)
check_function_exists(closelog IRCCD_HAVE_CLOSELOG)

if (NOT IRCCD_HAVE_SYSLOG_H OR NOT IRCCD_HAVE_OPENLOG OR NOT IRCCD_HAVE_CLOSELOG)
	set(IRCCD_HAVE_SYSLOG FALSE)
endif ()

# Check for struct stat fields.
check_struct_has_member("struct stat" st_atime sys/stat.h IRCCD_HAVE_STAT_ST_ATIME)
check_struct_has_member("struct stat" st_blksize sys/stat.h IRCCD_HAVE_STAT_ST_BLKSIZE)
check_struct_has_member("struct stat" st_blocks sys/stat.h IRCCD_HAVE_STAT_ST_BLOCKS)
check_struct_has_member("struct stat" st_ctime sys/stat.h IRCCD_HAVE_STAT_ST_CTIME)
check_struct_has_member("struct stat" st_dev sys/stat.h IRCCD_HAVE_STAT_ST_DEV)
check_struct_has_member("struct stat" st_gid sys/stat.h IRCCD_HAVE_STAT_ST_GID)
check_struct_has_member("struct stat" st_ino sys/stat.h IRCCD_HAVE_STAT_ST_INO)
check_struct_has_member("struct stat" st_mode sys/stat.h IRCCD_HAVE_STAT_ST_MODE)
check_struct_has_member("struct stat" st_mtime sys/stat.h IRCCD_HAVE_STAT_ST_MTIME)
check_struct_has_member("struct stat" st_nlink sys/stat.h IRCCD_HAVE_STAT_ST_NLINK)
check_struct_has_member("struct stat" st_rdev sys/stat.h IRCCD_HAVE_STAT_ST_RDEV)
check_struct_has_member("struct stat" st_size sys/stat.h IRCCD_HAVE_STAT_ST_SIZE)
check_struct_has_member("struct stat" st_uid sys/stat.h IRCCD_HAVE_STAT_ST_UID)

# Configuration file.
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/irccd)

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/internal/sysconfig.hpp.in
	${CMAKE_BINARY_DIR}/irccd/sysconfig.hpp
)

include_directories(
	${CMAKE_BINARY_DIR}
	${CMAKE_BINARY_DIR}/irccd
)

install(
	FILES ${CMAKE_BINARY_DIR}/irccd/sysconfig.hpp
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/irccd
)
