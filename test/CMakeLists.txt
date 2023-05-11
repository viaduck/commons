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

# remove the native wrapper that will be mocked in tests
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
list(REMOVE_ITEM COMMONS_NETWORK_FILES ${PARENT_DIR}/src/network/native/Native.cpp)

include(FindOrBuildGTest)
if (GTEST_FOUND)
    # collect test files
    file(GLOB_RECURSE TEST_BASE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/base/*.h ${CMAKE_CURRENT_SOURCE_DIR}/base/*.cpp)
    file(GLOB_RECURSE TEST_NETWORK_FILES ${CMAKE_CURRENT_SOURCE_DIR}/network/*.h ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp)

    macro(make_viaduck_test_target name)
        if (COMMONS_BASE_ONLY)
            add_executable(${name} ${COMMONS_BASE_FILES} ${TEST_BASE_FILES})
        else()
            add_executable(${name} ${COMMONS_BASE_FILES} ${COMMONS_NETWORK_FILES} ${TEST_BASE_FILES} ${TEST_NETWORK_FILES})
        endif()

        # includes
        target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
        # link deps
        target_link_libraries(${name} PUBLIC commons ${GTEST_TARGET})
        # enable additional warnings
        target_compile_options(${name} PRIVATE -Wall -Wextra)
    endmacro()
    make_viaduck_test_target(commons_test)

    include(EnableSanitizers)
    enable_sanitizers_for_target(commons_test)

    include(EnableCoverage)
    enable_coverage_for_target(commons_test ${CMAKE_CURRENT_SOURCE_DIR}/../
        "test/*" "external/*" "${CMAKE_BINARY_DIR}/*" "${GTEST_SRC_DIR}/*")
endif()
