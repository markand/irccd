# FindEv
# -----------
#
# Find Ev library, this modules defines:
#
# Ev_INCLUDE_DIRS, where to find ev.h
# Ev_LIBRARIES, where to find library
# Ev_FOUND, if it is found
#
# The following imported targets will be available:
#
# Ev::Ev, if found.
#

find_path(Ev_INCLUDE_DIR NAMES ev.h)
find_library(Ev_LIBRARY NAMES libev ev)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Ev
	FOUND_VAR Ev_FOUND
	REQUIRED_VARS Ev_LIBRARY Ev_INCLUDE_DIR
)

if (Ev_FOUND)
	set(Ev_LIBRARIES ${Ev_LIBRARY})
	set(Ev_INCLUDE_DIRS ${Ev_INCLUDE_DIR})

	if (NOT TARGET Ev::Ev)
		add_library(Ev::Ev UNKNOWN IMPORTED)
		set_target_properties(
			Ev::Ev
			PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${Ev_LIBRARY}"
				INTERFACE_INCLUDE_DIRECTORIES "${Ev_INCLUDE_DIRS}"
		)
	endif ()
endif ()

mark_as_advanced(Ev_INCLUDE_DIR Ev_LIBRARY)
