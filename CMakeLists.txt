# Copyright (C) 2015-2023 The ViaDuck Project
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

cmake_minimum_required(VERSION 3.13...3.27)
project(commons)

# options
option(COMMONS_BASE_ONLY "Skip compiling network code" OFF)
option(COMMONS_USE_LOCK_FREE_QUEUE "Enable lock-free instead of locking message queues" OFF)

# add additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/secure_memory/cmake-modules")

# platform specific thread library
find_package(Threads REQUIRED)

# deps
#  secure memory
add_subdirectory(external/secure_memory)
#  json
add_subdirectory(external/json)
#  compile flatbuffers lib to support cross compilation
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_INSTALL OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATC OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "" FORCE)
add_subdirectory(external/flatbuffers)
#  auto generated files
add_subdirectory(gen)

# create gen target
gen_add(commons ${CMAKE_CURRENT_SOURCE_DIR}/def/)

# collect files
file(GLOB_RECURSE COMMONS_BASE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/commons/*.cpp)
file(GLOB_RECURSE COMMONS_NETWORK_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/network/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp)

# this variable allows turning off all network and ssl dependencies
if (COMMONS_BASE_ONLY)
    # create library target using only base files
    add_library(commons STATIC ${COMMONS_BASE_FILES})
else()
    if (NOT TARGET ssl)
        add_subdirectory(external/openssl-cmake)
    endif()

    # create library target with all features
    add_library(commons STATIC ${COMMONS_BASE_FILES} ${COMMONS_NETWORK_FILES})
    # link to everything but the basics
    target_link_libraries(commons PUBLIC ssl crypto)
    if (WIN32)
        # ntoh related stuff on windows needs winsock
        target_link_libraries(commons PRIVATE ws2_32 crypt32)
    elseif(NOT ANDROID)
        target_link_libraries(commons PRIVATE Threads::Threads dl)
    endif()
    # OpenSSL related options
    if (SYSTEM_OPENSSL)
        target_compile_definitions(commons PUBLIC COMMONS_SYSTEM_OPENSSL)
    endif()
endif()

# includes for basic and network
target_include_directories(commons PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/external/concurrentqueue/
)
# link to base dependencies in any case
target_link_libraries(commons PUBLIC secure_memory commons_gen nlohmann_json)
# require and enable c++17 support
target_compile_features(commons PUBLIC cxx_std_17)
# compile options
target_compile_options(commons PRIVATE -Wall -Wextra)
target_compile_definitions(commons PUBLIC -DCOMMONS_USE_LOCK_FREE_QUEUE=$<BOOL:${COMMONS_USE_LOCK_FREE_QUEUE}>)

# tests
add_subdirectory(test)

# doxygen
include(Doxygen)
if (DOXYGEN_FOUND)
    setup_doxygen(commons_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
endif()
