project(libirccd)

set(
    HEADERS
    ${libirccd_SOURCE_DIR}/irccd/command.hpp
    ${libirccd_SOURCE_DIR}/irccd/config.hpp
    ${libirccd_SOURCE_DIR}/irccd/dynlib.hpp
    ${libirccd_SOURCE_DIR}/irccd/irccd.hpp
    ${libirccd_SOURCE_DIR}/irccd/plugin-dynlib.hpp
    ${libirccd_SOURCE_DIR}/irccd/plugin.hpp
    ${libirccd_SOURCE_DIR}/irccd/rule.hpp
    ${libirccd_SOURCE_DIR}/irccd/server.hpp
    ${libirccd_SOURCE_DIR}/irccd/service.hpp
    ${libirccd_SOURCE_DIR}/irccd/transport.hpp
)

set(
    SOURCES
    ${libirccd_SOURCE_DIR}/irccd/command.cpp
    ${libirccd_SOURCE_DIR}/irccd/config.cpp
    ${libirccd_SOURCE_DIR}/irccd/irccd.cpp
    ${libirccd_SOURCE_DIR}/irccd/plugin-dynlib.cpp
    ${libirccd_SOURCE_DIR}/irccd/rule.cpp
    ${libirccd_SOURCE_DIR}/irccd/server.cpp
    ${libirccd_SOURCE_DIR}/irccd/service.cpp
    ${libirccd_SOURCE_DIR}/irccd/transport.cpp
)

irccd_define_library(
    TARGET libirccd
    SOURCES
        ${libirccd_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES extern-ircclient libcommon
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccd_SOURCE_DIR}/irccd>
)
