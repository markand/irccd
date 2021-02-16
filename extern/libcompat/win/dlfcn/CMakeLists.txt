project(dl)
add_library(
	dl
	STATIC
	${dl_SOURCE_DIR}/dlfcn.h
	${dl_SOURCE_DIR}/dlfcn.c
)

# Correctly export the location of installed includes in the target
target_include_directories(
	dl
	PUBLIC
		$<BUILD_INTERFACE:${dl_SOURCE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (CMAKE_C_COMPILER_ID MATCHES "MSVC")
	target_compile_definitions(dl PRIVATE _CRT_SECURE_NO_WARNINGS)
endif ()

install(
	TARGETS dl
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
