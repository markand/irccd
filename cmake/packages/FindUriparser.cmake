# FindUriparser
# -------------
#
# Find liburiparser library, this modules defines:
#
# Uriparser_INCLUDE_DIRS, where to find uriparser/Uri.h
# Uriparser_LIBRARIES, where to find library
# Uriparser_FOUND, if it is found

find_path(Uriparser_INCLUDE_DIR NAMES uriparser/Uri.h)
find_library(Uriparser_LIBRARY NAMES liburiparser uriparser)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Uriparser
	FOUND_VAR Uriparser_FOUND
	REQUIRED_VARS Uriparser_LIBRARY Uriparser_INCLUDE_DIR
)

set(Uriparser_LIBRARIES ${Uriparser_LIBRARY})
set(Uriparser_INCLUDE_DIRS ${Uriparser_INCLUDE_DIR})

mark_as_advanced(Uriparser_INCLUDE_DIR Uriparser_LIBRARY)
