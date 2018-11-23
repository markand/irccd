#
# CMakeLists.txt -- CMake build system for irccd
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

project(windows)

#
# MinGW libraries.
# -------------------------------------------------------------------
#

if (MINGW)
	# debug libraries
	set(BOOST_CHRONO_DLL_DEBUG libboost_chrono-mt.dll)
	set(BOOST_DATE_TIME_DLL_DEBUG libboost_date_time-mt.dll)
	set(BOOST_FILESYSTEM_DLL_DEBUG libboost_filesystem-mt.dll)
	set(BOOST_SYSTEM_DLL_DEBUG libboost_system-mt.dll)
	set(BOOST_TIMER_DLL_DEBUG libboost_timer-mt.dll)
	set(BOOST_UNIT_TEST_FRAMEWORK_DLL_DEBUG libboost_unit_test_framework-mt.dll)
	set(CRYPTO_DLL_DEBUG libcrypto-1_1-x64.dll)
	set(SSL_DLL_DEBUG libssl-1_1-x64.dll)

	# MinGW specific.
	set(LIBGCC_DLL_DEBUG libgcc_s_seh-1.dll)
	set(LIBWINPTHREAD_DLL_DEBUG libwinpthread-1.dll)
	set(LIBSTDCPP_DLL_DEBUG libstdc++-6.dll)
	list(APPEND DLLS LIBGCC_DLL LIBWINPTHREAD_DLL LIBSTDCPP_DLL)

	# release libraries use the same.
	set(BOOST_CHRONO_DLL_RELEASE ${BOOST_CHRONO_DLL_DEBUG})
	set(BOOST_DATE_TIME_DLL_RELEASE ${BOOST_DATE_TIME_DLL_DEBUG})
	set(BOOST_FILESYSTEM_DLL_RELEASE ${BOOST_FILESYSTEM_DLL_DEBUG})
	set(BOOST_SYSTEM_DLL_RELEASE ${BOOST_SYSTEM_DLL_DEBUG})
	set(BOOST_TIMER_DLL_RELEASE ${BOOST_TIMER_DLL_DEBUG})
	set(BOOST_UNIT_TEST_FRAMEWORK_DLL_RELEASE ${BOOST_UNIT_TEST_FRAMEWORK_DLL_DEBUG})
	set(CRYPTO_DLL_RELEASE ${CRYPTO_DLL_DEBUG})
	set(SSL_DLL_RELEASE ${SSL_DLL_DEBUG})
	set(LIBGCC_DLL_RELEASE ${LIBGCC_DLL_DEBUG})
	set(LIBWINPTHREAD_DLL_RELEASE ${LIBWINPTHREAD_DLL_DEBUG})
	set(LIBSTDCPP_DLL_RELEASE ${LIBSTDCPP_DLL_DEBUG})
elseif (MSVC)
	# debug libraries
	set(BOOST_CHRONO_DLL_DEBUG boost_chrono-vc141-mt-gd-x64-1_68.dll)
	set(BOOST_DATE_TIME_DLL_DEBUG boost_date_time-vc141-mt-gd-x64-1_68.dll)
	set(BOOST_FILESYSTEM_DLL_DEBUG boost_filesystem-vc141-mt-gd-x64-1_68.dll)
	set(BOOST_SYSTEM_DLL_DEBUG boost_system-vc141-mt-gd-x64-1_68.dll)
	set(BOOST_TIMER_DLL_DEBUG boost_timer-vc141-mt-gd-x64-1_68.dll)
	set(BOOST_UNIT_TEST_FRAMEWORK_DLL_DEBUG boost_unit_test_framework-vc141-mt-gd-x64-1_68.dll)
	set(CRYPTO_DLL_DEBUG libcrypto-1_1-x64.dll)
	set(SSL_DLL_DEBUG libssl-1_1-x64.dll)

	# release libraries
	set(BOOST_CHRONO_DLL_RELEASE boost_chrono-vc141-mt-x64-1_68.dll)
	set(BOOST_DATE_TIME_DLL_RELEASE boost_date_time-vc141-mt-x64-1_68.dll)
	set(BOOST_FILESYSTEM_DLL_RELEASE boost_filesystem-vc141-mt-x64-1_68.dll)
	set(BOOST_SYSTEM_DLL_RELEASE boost_system-vc141-mt-x64-1_68.dll)
	set(BOOST_TIMER_DLL_RELEASE boost_timer-vc141-mt-x64-1_68.dll)
	set(BOOST_UNIT_TEST_FRAMEWORK_DLL_RELEASE boost_unit_test_framework-vc141-mt-x64-1_68.dll)
	set(CRYPTO_DLL_RELEASE libcrypto-1_1-x64.dll)
	set(SSL_DLL_RELEASE libssl-1_1-x64.dll)
endif ()

list(
	APPEND
	DLLS
	BOOST_CHRONO_DLL
	BOOST_DATE_TIME_DLL
	BOOST_FILESYSTEM_DLL
	BOOST_SYSTEM_DLL
	BOOST_UNIT_TEST_FRAMEWORK_DLL
)

if (IRCCD_HAVE_SSL)
	list(APPEND DLLS CRYPTO_DLL SSL_DLL)
endif ()
if (IRCCD_HAVE_JS)
	list(APPEND DLLS BOOST_TIMER_DLL)
endif ()

#
# Find DLL to copy/install by iterating DLLS value.
#
# Creates a cache IRCCD_WITH_<NAME> variable with the path to the DLL if found,
# otherwise, set to not found.
#
# If found, IRCCD_WITH_<NAME> is appended to IRCCD_DLLS global variable and set
# as parent scope.
#
# If not found, IRCCD_WITH_<NAME> is appended to IRCCD_DLLS_NOT_FOUND variable.
#

foreach (name ${DLLS})
	find_program(
		IRCCD_WITH_${name}_DEBUG
		NAMES ${${name}_DEBUG}
		DOC "Path to ${name} DLL (Debug)"
	)
	find_program(
		IRCCD_WITH_${name}_RELEASE
		NAMES ${${name}_RELEASE}
		DOC "Path to ${name} DLL (Release)"
	)

	if (EXISTS ${IRCCD_WITH_${name}_DEBUG})
		install(
			PROGRAMS
				$<$<CONFIG:Debug>:${IRCCD_WITH_${name}_DEBUG}>
			COMPONENT libirccd
			DESTINATION bin
		)
		list(APPEND FOUND IRCCD_WITH_${name}_DEBUG)
	else ()
		list(APPEND NOT_FOUND IRCCD_WITH_${name}_DEBUG)
	endif ()

	if (EXISTS ${IRCCD_WITH_${name}_RELEASE})
		install(
			PROGRAMS
				$<$<CONFIG:Release>:${IRCCD_WITH_${name}_RELEASE}>
			COMPONENT libirccd
			DESTINATION bin
		)
		list(APPEND FOUND IRCCD_WITH_${name}_RELEASE)
	else ()
		list(APPEND NOT_FOUND IRCCD_WITH_${name}_RELEASE)
	endif ()
endforeach ()

setg(IRCCD_DLLS "${FOUND}")
setg(IRCCD_DLLS_NOT_FOUND "${NOT_FOUND}")
