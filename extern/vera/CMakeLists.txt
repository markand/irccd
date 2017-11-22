project(vera++)
cmake_minimum_required(VERSION 3.0)

set(VERA_MAJOR 1)
set(VERA_MINOR 3)
set(VERA_BUILD 0)
set(VERA_VERSION "${VERA_MAJOR}.${VERA_MINOR}.${VERA_BUILD}")

add_subdirectory(src)
