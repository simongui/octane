cmake_minimum_required(VERSION 2.6)
include(CMakeToolsHelpers OPTIONAL)
include("common.cmake")

# ----------------------------------------
# libchannels
# ----------------------------------------
project(libchannels C)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_definitions(-std=gnu99)
add_definitions(-msse4.1)
add_definitions(-pedantic)
add_definitions(-O3)
add_definitions(-Wall)
add_definitions(-Wextra)
add_definitions(-Wcast-align)
add_definitions(-w)

if (UNIX)
    add_definitions(-DUNIX)
endif (UNIX)

file(GLOB_RECURSE LIBCHANNELS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/channels/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/channels/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/channels/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/channels/*.cpp)

list(SORT LIBCHANNELS_SOURCES)
create_source_group("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/src" ${LIBCHANNELS_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/lib/libuv/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_library(libchannels STATIC ${LIBCHANNELS_SOURCES})

target_link_libraries (libchannels ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_SOURCE_DIR}/lib/libuv/.libs/libuv.a)

GET_PROPERTY(libchannels_location TARGET libchannels PROPERTY LOCATION)

# ----------------------------------------
# Hello world sample
# ----------------------------------------
file(GLOB_RECURSE HELLOWORLD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hello_world/*.cpp)

list(SORT HELLOWORLD_SOURCES)
create_source_group("Source Files" "${CMAKE_CURRENT_SOURCE_DIR}/src" ${HELLOWORLD_SOURCES})
include_directories(${CMAKE_SOURCE_DIR}/lib/libuv/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable (hello_world
    ${HELLOWORLD_SOURCES})

add_dependencies(hello_world libchannels)

# Libraries to link in reverse order because that's what ld requires.
target_link_libraries (hello_world
    ${libchannels_location}
    ${CMAKE_SOURCE_DIR}/lib/libuv/.libs/libuv.a
    ${CMAKE_THREAD_LIBS_INIT})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries (hello_world rt)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
