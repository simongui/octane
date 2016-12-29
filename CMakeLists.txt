cmake_minimum_required(VERSION 2.6)
include(CMakeToolsHelpers OPTIONAL)
include("common.cmake")

set (CMAKE_CXX_STANDARD 14)

# ----------------------------------------
# octane
# ----------------------------------------
project(octane C CXX)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

#add_definitions(-std=gnu99)
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
include_directories(${CMAKE_SOURCE_DIR}/lib/rapidjson/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable (hello_world
    ${HELLOWORLD_SOURCES})

add_dependencies(hello_world octane)

# Libraries to link in reverse order because that's what ld requires.
target_link_libraries (hello_world
    ${octane_location}
    ${CMAKE_SOURCE_DIR}/lib/libuv/.libs/libuv.a
    ${CMAKE_THREAD_LIBS_INIT})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries (hello_world rt)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
