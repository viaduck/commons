# Copyright (C) 2015-2019 The ViaDuck Project
#
# This file is part of Commons.
#
# Commons is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Commons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Commons.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.1)
project(Commons)

# add additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/securememory/cmake-modules")

# python used to auto generate files
find_package(PythonInterp 3.4 REQUIRED)
# platform specific thread library
find_package(Threads REQUIRED)

# enable and require c++14 support
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# options
option(COMMONS_BASE_ONLY "Skip compiling network code" OFF)
option(COMMONS_USE_LOCK_FREE_QUEUE "Enable lock-free instead of locking message queues" OFF)

# externals
add_subdirectory(external/securememory)

# compile flatbuffers lib to support cross compilation
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_INSTALL OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATC OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "" FORCE)
add_subdirectory(external/flatbuffers)

# auto generated files
add_subdirectory(gen)

# create gen target
gen_add(Commons ${CMAKE_CURRENT_SOURCE_DIR}/def/)

# collect files
file(GLOB_RECURSE COMMONS_BASE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/commons/*.cpp)
file(GLOB_RECURSE COMMONS_BASE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/*.h)
file(GLOB_RECURSE COMMONS_NETWORK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp)
file(GLOB_RECURSE COMMONS_NETWORK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/network/*.h)

# convenience
set(COMMONS_BASIC_FILES ${COMMONS_BASE_SOURCES} ${COMMONS_BASE_HEADERS})
set(COMMONS_NETWORK_FILES ${COMMONS_NETWORK_SOURCES} ${COMMONS_NETWORK_HEADERS})

# this variable allows turning off all network and ssl dependencies
if (COMMONS_BASE_ONLY)

    # create the library using only base files
    add_library(Commons STATIC ${COMMONS_BASIC_FILES})

else()

    # require openssl
    add_definitions(-DOPENSSL_API_COMPAT=0x10100000L)       # OpenSSL API compatibility
    if (NOT TARGET ssl)
        add_subdirectory(external/openssl-cmake)
    endif()
    if (SYSTEM_OPENSSL)
        add_definitions(-DSYSTEM_OPENSSL)
    endif()

    # build
    add_library(Commons STATIC ${COMMONS_BASIC_FILES} ${COMMONS_NETWORK_FILES})

    # link to everything but the basics
    target_link_libraries(Commons PUBLIC ssl crypto)

    # ntoh related stuff on windows needs winsock
    if (WIN32)
        target_link_libraries(Commons PUBLIC ws2_32 crypt32)
    elseif(NOT ANDROID)
        target_link_libraries(Commons PUBLIC Threads::Threads dl)
    endif()

    # doxygen
    include(Doxygen)
    if (DOXYGEN_FOUND)
        setup_doxygen(Commons_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    endif()
endif()

# tests
add_subdirectory(test)

# link to base dependencies in any case
# and force a rebuild of Commons if one of the spec files changes
target_link_libraries(Commons PUBLIC SecureMemory Commons_gen)

# export threading dependency because it is used in the header
target_include_directories(Commons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/concurrentqueue/)

# export the basic includes and compile options in any case
target_include_directories(Commons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_options(Commons PUBLIC -Wall -Wextra)
target_compile_definitions(Commons PUBLIC -DCOMMONS_USE_LOCK_FREE_QUEUE=$<BOOL:${COMMONS_USE_LOCK_FREE_QUEUE}>)
