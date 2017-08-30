# remove the native wrapper that will be mocked in tests
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
list(REMOVE_ITEM SOURCE_FILES ${PARENT_DIR}/src/commons/network/NativeWrapper.cpp)

# find system gtest
include(FindOrBuildGTest)

if (GTEST_FOUND)
    file(GLOB_RECURSE TEST_FILES_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    file(GLOB_RECURSE TEST_FILES_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

    ### coverage
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fexceptions")

    add_definitions(-DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ed25519_tests.txt ${CMAKE_CURRENT_BINARY_DIR}/ed25519_tests.txt COPYONLY)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${GTEST_INCLUDE_DIRS})

    ###
    add_executable(VDCommons_Test ${SOURCE_FILES} ${VDCOMMONS_HEADERS} ${CURVE25519_SOURCES} ${TEST_FILES_SOURCES} ${TEST_FILES_HEADERS})
    add_dependencies(VDCommons_Test protocol_generation)
    add_dependencies(VDCommons_Test enum_generation)
    target_link_libraries(VDCommons_Test ${GTEST_BOTH_LIBRARIES} pthread VDCommons)

    ## code coverage
    include(CodeCoverage)
    if (LCOV_PATH AND NOT ANDROID)
        setup_target_for_coverage(VDCommons_Coverage VDCommons_Test ${CMAKE_CURRENT_BINARY_DIR}/coverage "'${CMAKE_CURRENT_SOURCE_DIR}/*';'${CMAKE_SOURCE_DIR}/external/*'")
    endif()

endif()