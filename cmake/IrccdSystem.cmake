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

#
# Recent versions of CMake has nice C++ feature detection for modern
# C++ but they are still a bit buggy so we use this
# instead.
#
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    #
    # For GCC, we require at least GCC 5.1
    #
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.1")
        message(FATAL_ERROR "You need at least GCC 5.1")
    endif ()

    set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14 ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #
    # LLVM/clang implemented C++14 starting from version 3.4 but the
    # switch -std=c++14 was not available.
    #
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.4")
        message(FATAL_ERROR "You need at least Clang 3.4")
    endif ()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "3.5")
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++1y ${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++14 ${CMAKE_CXX_FLAGS}")
    endif ()
elseif (MSVC14)
    set(CMAKE_C_FLAGS "/DWIN32_LEAN_AND_MEAN /DNOMINMAX /wd4267 /wd4800 /D_CRT_SECURE_NO_WARNINGS ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "/DWIN32_LEAN_AND_MEAN /DNOMINMAX /wd4267 /wd4800 /D_CRT_SECURE_NO_WARNINGS /EHsc ${CMAKE_CXX_FLAGS}")
else ()
    message(WARNING "Unsupported ${CMAKE_CXX_COMPILER_ID}, may not build correctly.")
endif ()

if (MINGW)
    set(CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0600 ${CMAKE_CXX_FLAGS}")
endif ()

if (CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(IRCCD_64BITS TRUE)
else ()
    set(IRCCD_64BITS FALSE)
endif ()

#
# System identification.
# -------------------------------------------------------------------
#

if (WIN32)
    set(IRCCD_SYSTEM_WINDOWS TRUE)
elseif (APPLE)
    set(IRCCD_SYSTEM_MAC TRUE)
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(IRCCD_SYSTEM_FREEBSD TRUE)
elseif (CMAKE_SYSTEM_NAME MATCHES "DragonFly")
    set(IRCCD_SYSTEM_DRAGONFLYBSD TRUE)
elseif (CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    set(IRCCD_SYSTEM_NETBSD TRUE)
elseif (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    set(IRCCD_SYSTEM_OPENBSD TRUE)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(IRCCD_SYSTEM_LINUX TRUE)
endif ()

#
# Portability requirements.
# -------------------------------------------------------------------
#

check_type_size(int8_t HAVE_INT8)
check_type_size(uint8_t HAVE_UINT8)
check_type_size(int16_t HAVE_INT16)
check_type_size(uint16_t HAVE_UINT16)
check_type_size(int32_t HAVE_INT32)
check_type_size(uint32_t HAVE_UINT32)
check_type_size(int64_t HAVE_INT64)
check_type_size(uint64_t HAVE_UINT64)

if (NOT HAVE_STDINT_H)
    message("irccd requires stdint.h or cstdint header")
endif ()

#
# Where any of these function / feature is required, include the <irccd/sysconfig.h> file.
#
# The following variables are defined in irccd/sysconfig.h
#
# HAVE_ACCESS           True if has access(2) function (and sys/types.h and sys/stat.h),
# HAVE_DAEMON           True if daemon(3),
# HAVE_GETLOGIN         True if getlogin(3) function (and unistd.h)
# HAVE_GETPID           True if has getpid(2) function (and sys/types.h and unistd.h and grp.h),
# HAVE_POPEN            True if has popen(3) function (in stdio.h)
# HAVE_SETGID           True if has setgid(2) function and getgrnam(3) (and sys/types.h and unistd.h and pwd.h),
# HAVE_SETPROGNAME      True if setprogname(3) is available from C library,
# HAVE_SETUID           True if has setuid(2) function and getpwnam(3) (and sys/types.h and unistd.h and pwd.h),
# HAVE_STAT             True if has stat(2) function (and sys/types.h and sys/stat.h),
# HAVE_STAT_ST_DEV      The struct stat has st_dev field,
# HAVE_STAT_ST_INO      The struct stat has st_ino field,
# HAVE_STAT_ST_NLINK    The struct stat has st_nlink field,
# HAVE_STAT_ST_UID      The struct stat has st_uid field,
# HAVE_STAT_ST_GID      The struct stat has st_gid field,
# HAVE_STAT_ST_ATIME    The struct stat has st_atime field,
# HAVE_STAT_ST_MTIME    The struct stat has st_mtime field,
# HAVE_STAT_ST_CTIME    The struct stat has st_ctime field,
# HAVE_STAT_ST_SIZE     The struct stat has st_size field,
# HAVE_STAT_ST_BLKSIZE  The struct stat has st_blksize field,
# HAVE_STAT_ST_BLOCKS   The struct stat has st_blocks field,
# HAVE_SYSLOG           True if syslog functions are available (and syslog.h),
#

# Check for unistd.h
check_include_file(unistd.h HAVE_UNISTD_H)

# Check for sys/types.h
check_include_file(sys/types.h HAVE_SYS_TYPES_H)

# Check for daemon(3) function, include:
#
# #include <cstdlib>
check_function_exists(daemon HAVE_DAEMON)

# Check of setprogname(3) function, include:
#
# #include <cstdlib>
check_function_exists(setprogname HAVE_SETPROGNAME)

# access() POSIX function
#
# If HAVE_ACCESS is defined, include:
#
# #include <unistd.h>
check_function_exists(access HAVE_ACCESS)

if (NOT HAVE_UNISTD_H)
    set(HAVE_ACCESS FALSE)
endif ()

# getlogin() function
#
# If HAVE_GETLOGIN is defined, include:
#
# #include <unistd.h>
check_function_exists(getlogin HAVE_GETLOGIN)

if (NOT HAVE_UNISTD_H)
    set(HAVE_GETLOGIN FALSE)
endif ()

# getpid() function
#
# If HAVE_GETPID is defined, include:
#
# #include <sys/types.h>
# #include <unistd.h>
check_function_exists(getpid HAVE_GETPID)

if (NOT HAVE_UNISTD_H OR NOT HAVE_SYS_TYPES_H)
    set(HAVE_GETPID FALSE)
endif ()

# setgid() function (and getgrnam)
#
# If HAVE_SETGID is defined, include:
#
# #include <sys/types.h>
# #include <unistd.h>
# #include <grp.h>        // only for getgrnam
check_include_file(grp.h HAVE_GRP_H)
check_function_exists(getgrnam HAVE_GETGRNAM)
check_function_exists(setgid HAVE_SETGID)

if (NOT HAVE_UNISTD_H OR NOT HAVE_SYS_TYPES_H OR NOT HAVE_GETGRNAM OR NOT HAVE_GRP_H)
    set(HAVE_SETGID FALSE)
endif ()

# popen() function
#
# If HAVE_POPEN is defined, include;
#
# #include <cstdio>
check_function_exists(popen HAVE_POPEN)

# setuid() function (and getpwnam)
#
# If HAVE_SETUID is defined, include:
#
# #include <sys/types.h>
# #include <unistd.h>
# #include <pwd.h>        // only for getpwnam
check_include_file(pwd.h HAVE_PWD_H)
check_function_exists(getpwnam HAVE_GETPWNAM)
check_function_exists(setuid HAVE_SETUID)

if (NOT HAVE_UNISTD_H OR NOT HAVE_SYS_TYPES_H OR NOT HAVE_GETPWNAM OR NOT HAVE_PWD_H)
    set(HAVE_SETUID FALSE)
endif ()

# stat(2) function
#
# If HAVE_STAT is defined, include:
#
# #include <sys/types.h>
# #include <sys/stat.h>
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_function_exists(stat HAVE_STAT)

# If the sys/stat.h is not found, we disable stat(2)
if (NOT HAVE_SYS_STAT_H OR NOT HAVE_SYS_TYPES_H)
    set(HAVE_STAT FALSE)
endif ()

# syslog functions
#
# If HAVE_SYSLOG is defined, include:
#
# #include <syslog.h>
check_include_file(syslog.h HAVE_SYSLOG_H)
check_function_exists(openlog HAVE_OPENLOG)
check_function_exists(syslog HAVE_SYSLOG)
check_function_exists(closelog HAVE_CLOSELOG)

if (NOT HAVE_SYSLOG_H OR NOT HAVE_OPENLOG OR NOT HAVE_CLOSELOG)
    set(HAVE_SYSLOG FALSE)
endif ()

# Check for struct stat fields.
check_struct_has_member("struct stat" st_atime sys/stat.h HAVE_STAT_ST_ATIME)
check_struct_has_member("struct stat" st_blksize sys/stat.h HAVE_STAT_ST_BLKSIZE)
check_struct_has_member("struct stat" st_blocks sys/stat.h HAVE_STAT_ST_BLOCKS)
check_struct_has_member("struct stat" st_ctime sys/stat.h HAVE_STAT_ST_CTIME)
check_struct_has_member("struct stat" st_dev sys/stat.h HAVE_STAT_ST_DEV)
check_struct_has_member("struct stat" st_gid sys/stat.h HAVE_STAT_ST_GID)
check_struct_has_member("struct stat" st_ino sys/stat.h HAVE_STAT_ST_INO)
check_struct_has_member("struct stat" st_mode sys/stat.h HAVE_STAT_ST_MODE)
check_struct_has_member("struct stat" st_mtime sys/stat.h HAVE_STAT_ST_MTIME)
check_struct_has_member("struct stat" st_nlink sys/stat.h HAVE_STAT_ST_NLINK)
check_struct_has_member("struct stat" st_rdev sys/stat.h HAVE_STAT_ST_RDEV)
check_struct_has_member("struct stat" st_size sys/stat.h HAVE_STAT_ST_SIZE)
check_struct_has_member("struct stat" st_uid sys/stat.h HAVE_STAT_ST_UID)

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
