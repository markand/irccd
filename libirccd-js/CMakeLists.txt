project(libirccd-js)

set(
    HEADERS
    ${libirccd-js_SOURCE_DIR}/irccd/duktape.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-directory.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-elapsed-timer.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-file.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-irccd.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-logger.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-plugin.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-server.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-system.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-timer.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/module.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-unicode.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-util.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/plugin-js.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/timer.hpp
    ${libirccd-js_SOURCE_DIR}/irccd/unicode.hpp
)

set(
    SOURCES
    ${libirccd-js_SOURCE_DIR}/irccd/mod-directory.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-elapsed-timer.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-file.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-irccd.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-logger.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-plugin.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-server.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-system.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-timer.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-unicode.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/mod-util.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/plugin-js.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/timer.cpp
    ${libirccd-js_SOURCE_DIR}/irccd/unicode.cpp
)

irccd_define_library(
    TARGET libirccd-js
    SOURCES
        ${libirccd-js_SOURCE_DIR}/CMakeLists.txt
        ${HEADERS}
        ${SOURCES}
    LIBRARIES extern-duktape libirccd
)
