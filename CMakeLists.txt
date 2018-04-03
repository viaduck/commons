# Copyright (C) 2015-2018 The ViaDuck Project
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

# enable and require c++14 support
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# externals
add_subdirectory(external/securememory)

# collect files
file(GLOB_RECURSE COMMONS_BASE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/commons/*.cpp)
file(GLOB_RECURSE COMMONS_BASE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/*.h)
file(GLOB_RECURSE COMMONS_NETWORK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp)
file(GLOB_RECURSE COMMONS_NETWORK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/network/*.h)
file(GLOB_RECURSE COMMONS_CURVE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/curve25519/*.cpp)
file(GLOB_RECURSE COMMONS_CURVE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/curve25519/*.h)

# convenience
set(COMMONS_BASIC_FILES ${COMMONS_BASE_SOURCES} ${COMMONS_BASE_HEADERS})
set(COMMONS_NETWORK_FILES ${COMMONS_NETWORK_SOURCES} ${COMMONS_NETWORK_HEADERS})
set(COMMONS_CURVE_FILES ${COMMONS_CURVE_SOURCES} ${COMMONS_CURVE_HEADERS})

# this variable allows turning off all network, ssl, and curve dependencies
if (COMMONS_BASE_ONLY)

    # create the library using only base files
    add_library(Commons STATIC ${COMMONS_BASIC_FILES})

else()

    # require openssl
    add_definitions(-DOPENSSL_API_COMPAT=0x10100000L)       # OpenSSL API compatibility
    if (NOT TARGET ssl)
        add_subdirectory(external/openssl-cmake)
    endif()

    # build
    add_library(Commons STATIC ${COMMONS_BASIC_FILES} ${COMMONS_NETWORK_FILES})
    add_library(curve25519 STATIC ${COMMONS_CURVE_FILES})

    # link to everything but the basics
    target_link_libraries(curve25519 ssl crypto)
    target_link_libraries(Commons ssl crypto curve25519)

    # ntoh related stuff on windows needs winsock
    if (WIN32)
        target_link_libraries(Commons ws2_32)
    endif()

    # export all includes but the basics
    target_include_directories(Commons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/curve25519/)
    target_include_directories(curve25519 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

    # doxygen
    include(Doxygen)
    if (DOXYGEN_FOUND)
        setup_doxygen(Commons_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    endif()
endif()

# tests
add_subdirectory(test)
# auto generated files
add_subdirectory(gen)

# link to base dependencies in any case
# and force a rebuild of Commons if one of the spec files changes
target_link_libraries(Commons SecureMemory auto_gen)

# export the basic includes and compile options in any case
target_include_directories(Commons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_options(Commons PUBLIC -Wall -Wextra)
