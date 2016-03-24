project(lib)

include(irccd/CMakeSources.cmake)

# TMP
add_library(libirccd ${HEADERS} ${SOURCES})

target_include_directories(
	libirccd
	PUBLIC
		${irccd_BINARY_DIR}
		${OPENSSL_INCLUDE_DIR}
		${lib_SOURCE_DIR}
)

if (IRCCD_SYSTEM_WINDOWS)
	list(APPEND LIBRARIES ws2_32 shlwapi)
elseif (IRCCD_SYSTEM_MAC)
	list(APPEND LIBRARIES resolv)
endif ()

target_link_libraries(libirccd extern-duktape extern-ircclient extern-jansson ${LIBRARIES})
set_target_properties(libirccd PROPERTIES PREFIX "")

# TODO: install with export stuff and all
