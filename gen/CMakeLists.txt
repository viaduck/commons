# Copyright (C) 2018-2025 The ViaDuck Project
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

cmake_minimum_required(VERSION 3.1...3.27)

# python used to auto generate files
find_package(Python3 3.4 REQUIRED)

# provide flatbuffers compiler
if (SYSTEM_FLATC)
    # use system provided flatc
    find_program(FLATC_PROGRAM flatc)
    if (NOT FLATC_PROGRAM)
        message(FATAL_ERROR "Could not find flatc. Please install flatbuffers.")
    endif()
else()
    # download prebuilt flatc

    # detect host system
    if (CMAKE_HOST_WIN32)
        set(GEN_FLATBUFFER_ARCH x86_64-windows)
    elseif (CMAKE_HOST_APPLE)
        set(GEN_FLATBUFFER_ARCH x86_64-osx)
    elseif (CMAKE_HOST_UNIX)
        set(GEN_FLATBUFFER_ARCH x86_64-linux)
    endif()
    
    if (NOT FLATC_VERSION)
        set(FLATC_VERSION "25.2.10")
    endif()

    # only download if needed
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz)
        set(PREBUILT_URL https://builds.viaduck.org/prebuilts/flatbuffers/${FLATC_VERSION}/${GEN_FLATBUFFER_ARCH}.tar.gz)
        file(DOWNLOAD ${PREBUILT_URL} ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    set(FLATC_PROGRAM ${CMAKE_CURRENT_BINARY_DIR}/${GEN_FLATBUFFER_ARCH}/bin/flatc CACHE INTERNAL "" FORCE)
endif()

# output directory
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
# cache to extend scope
set(Python3_EXECUTABLE ${Python3_EXECUTABLE} CACHE INTERNAL "" FORCE)

function(gen_add TARGET DEF_DIR)
    set(GEN_INCLUDES ${CMAKE_BINARY_DIR}/include)

    # get list of headers that need to be generated
    execute_process(COMMAND ${Python3_EXECUTABLE}
        ${GEN_DIR}/generator.py list ${DEF_DIR} ${GEN_INCLUDES}
        WORKING_DIRECTORY ${GEN_DIR}
        OUTPUT_VARIABLE GEN_HEADERS
        RESULT_VARIABLE GEN_RESULT
    )
    if (GEN_RESULT AND NOT GEN_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed generating header list with: ${GEN_RESULT}")
    endif()

    # get list of source files to depend on
    execute_process(COMMAND ${Python3_EXECUTABLE}
        ${GEN_DIR}/generator.py depend ${DEF_DIR} ${GEN_INCLUDES}
        WORKING_DIRECTORY ${GEN_DIR}
        OUTPUT_VARIABLE GEN_DEPENDS
        RESULT_VARIABLE GEN_RESULT
    )
    if (GEN_RESULT AND NOT GEN_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed generating depends list with: ${GEN_RESULT}")
    endif()

    # custom command for actually generating the headers using the spec files
    add_custom_command(
        OUTPUT ${GEN_HEADERS}
        COMMAND ${Python3_EXECUTABLE}
        ${GEN_DIR}/generator.py generate ${DEF_DIR} ${GEN_INCLUDES} ${FLATC_PROGRAM}
        WORKING_DIRECTORY ${GEN_DIR}
        DEPENDS ${GEN_DEPENDS}
    )

    # add targets
    add_custom_target(${TARGET}_gen_files ALL DEPENDS ${GEN_HEADERS})

    # fake target for linking and source files
    add_library(${TARGET}_gen INTERFACE)
    add_dependencies(${TARGET}_gen ${TARGET}_gen_files)

    # include autogen headers
    target_include_directories(${TARGET}_gen INTERFACE ${GEN_INCLUDES} ${GEN_DIR}/../external/flatbuffers/include)
endfunction()
