project(libirccdctl)

set(
    HEADERS
    ${libirccdctl_SOURCE_DIR}/irccd/client.hpp
    ${libirccdctl_SOURCE_DIR}/irccd/irccdctl.hpp
)

set(
    SOURCES
    ${libirccdctl_SOURCE_DIR}/irccd/client.cpp
)

irccd_define_library(
    TARGET libirccdctl
    SOURCES
        ${libirccdctl_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES libcommon
    PUBLIC_INCLUDES ${libirccdctl_SOURCE_DIR}/irccd
)
