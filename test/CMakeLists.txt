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

# remove the native wrapper that will be mocked in tests
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
list(REMOVE_ITEM COMMONS_NETWORK_FILES ${PARENT_DIR}/src/network/native/Native.cpp)

# find system gtest
include(FindOrBuildGTest)

if (GTEST_FOUND)
    file(GLOB_RECURSE TEST_BASE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/base/*.cpp)
    file(GLOB_RECURSE TEST_BASE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/base/*.h)
    file(GLOB_RECURSE TEST_NETWORK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp)
    file(GLOB_RECURSE TEST_NETWORK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/network/*.h)

    set(TEST_BASE_FILES ${TEST_BASE_SOURCES} ${TEST_BASE_HEADERS})
    set(TEST_NETWORK_FILES ${TEST_NETWORK_SOURCES} ${TEST_NETWORK_HEADERS})

    ### coverage
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fexceptions")

    add_definitions(-DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${GTEST_INCLUDE_DIRS})

    ###
    if (COMMONS_BASE_ONLY)
        add_executable(Commons_Test ${COMMONS_BASE_FILES} ${TEST_BASE_SOURCES} ${TEST_BASE_HEADERS})
    else()
        add_executable(Commons_Test ${COMMONS_BASE_FILES} ${COMMONS_NETWORK_FILES} ${TEST_BASE_FILES} ${TEST_NETWORK_FILES})
    endif()

    target_link_libraries(Commons_Test Commons Commons_gen ${GTEST_BOTH_LIBRARIES})

    ## code coverage
    include(CodeCoverage)
    if (LCOV_PATH AND NOT ANDROID)
        setup_target_for_coverage(Commons_Coverage Commons_Test ${CMAKE_CURRENT_BINARY_DIR}/coverage "'${CMAKE_CURRENT_SOURCE_DIR}/*';'${CMAKE_SOURCE_DIR}/external/*';'${CMAKE_SOURCE_DIR}/build/*'")
    endif()

endif()
