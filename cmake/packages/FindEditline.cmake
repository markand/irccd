# FindEditline
# ------------
#
# Find libedit library, this modules defines:
#
# Editline_INCLUDE_DIRS, where to find histedit.h
# Editline_LIBRARIES, where to find library
# Editline_FOUND, if it is found

find_path(Editline_INCLUDE_DIR NAMES histedit.h)
find_library(Editline_LIBRARY NAMES libedit edit)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Editline
	FOUND_VAR Editline_FOUND
	REQUIRED_VARS Editline_LIBRARY Editline_INCLUDE_DIR
)

set(Editline_LIBRARIES ${Editline_LIBRARY})
set(Editline_INCLUDE_DIRS ${Editline_INCLUDE_DIR})

mark_as_advanced(Editline_INCLUDE_DIR Editline_LIBRARY)
