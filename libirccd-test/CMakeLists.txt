project(libirccd-test)

irccd_define_library(
    TARGET libirccd-test
    SOURCES
        ${libirccd-test_SOURCE_DIR}/irccd/command-tester.cpp
        ${libirccd-test_SOURCE_DIR}/irccd/command-tester.hpp
        ${libirccd-test_SOURCE_DIR}/irccd/server-tester.cpp
        ${libirccd-test_SOURCE_DIR}/irccd/server-tester.hpp
    LIBRARIES libirccd libirccdctl extern-gtest
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${libirccd-test_SOURCE_DIR}/irccd>
)
