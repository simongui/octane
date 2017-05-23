cmake_minimum_required(VERSION 2.6)
include(CMakeToolsHelpers OPTIONAL)
include("common.cmake")

# ----------------------------------------
# Octane
# ----------------------------------------
project(octane C CXX)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set (CMAKE_CXX_STANDARD 14)

add_definitions(-std=gnu99)
#add_definitions(-msse4.1)
add_definitions(-mavx)
add_definitions(-pedantic)
add_definitions(-O3)
add_definitions(-Wall)
add_definitions(-Wextra)
add_definitions(-Wcast-align)
add_definitions(-w)

if (UNIX)
    add_definitions(-DUNIX)
endif (UNIX)

file(GLOB_RECURSE OCTANE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/octane/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/octane/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/octane/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/octane/*.cpp)

list(SORT OCTANE_SOURCES)
create_source_group("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/src" ${OCTANE_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/lib/libuv/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_library(octane STATIC ${OCTANE_SOURCES})

target_link_libraries (octane ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_SOURCE_DIR}/lib/libuv/.libs/libuv.a)

GET_PROPERTY(octane_location TARGET octane PROPERTY LOCATION)

# ----------------------------------------
# Tests
# ----------------------------------------
file(GLOB_RECURSE TESTS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/*.cpp)

list(SORT TESTS_SOURCES)
create_source_group("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/src" ${TESTS_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/lib/libuv/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable (tests
        ${TESTS_SOURCES})

add_dependencies(tests octane)

# Libraries to link in reverse order because that's what ld requires.
target_link_libraries (tests
        ${octane_location}
        ${CMAKE_SOURCE_DIR}/lib/libuv/.libs/libuv.a
        ${CMAKE_THREAD_LIBS_INIT})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries (tests rt)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# ----------------------------------------
# Techempower benchmarks
# ----------------------------------------
file(GLOB_RECURSE TECHEMPOWER_BENCHMARKS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/techempower_benchmarks/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/techempower_benchmarks/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/techempower_benchmarks/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/techempower_benchmarks/*.cpp)

list(SORT TECHEMPOWER_BENCHMARKS_SOURCES)
create_source_group("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/src" ${TECHEMPOWER_BENCHMARKS_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/lib/libuv/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/rapidjson/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable (techempower_benchmarks
    ${TECHEMPOWER_BENCHMARKS_SOURCES})

add_dependencies(techempower_benchmarks octane)

# Libraries to link in reverse order because that's what ld requires.
target_link_libraries (techempower_benchmarks
    ${octane_location}
    ${CMAKE_SOURCE_DIR}/lib/libuv/.libs/libuv.a
    ${CMAKE_THREAD_LIBS_INIT})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries (techempower_benchmarks rt)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
