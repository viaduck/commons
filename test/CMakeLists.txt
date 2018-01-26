# remove the native wrapper that will be mocked in tests
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
list(REMOVE_ITEM COMMONS_NETWORK_FILES ${PARENT_DIR}/src/network/NativeWrapper.cpp)

# find system gtest
include(FindOrBuildGTest)

if (GTEST_FOUND)
    file(GLOB_RECURSE TEST_BASE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/base/*.cpp)
    file(GLOB_RECURSE TEST_BASE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/base/*.h)
    file(GLOB_RECURSE TEST_NETWORK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp)
    file(GLOB_RECURSE TEST_NETWORK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/network/*.h)
    file(GLOB_RECURSE TEST_CURVE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/curve25519/*.cpp)
    file(GLOB_RECURSE TEST_CURVE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/curve25519/*.h)

    set(TEST_BASE_FILES ${TEST_BASE_SOURCES} ${TEST_BASE_HEADERS})
    set(TEST_NETWORK_FILES ${TEST_NETWORK_SOURCES} ${TEST_NETWORK_HEADERS})
    set(TEST_CURVE_FILES ${TEST_CURVE_SOURCES} ${TEST_CURVE_HEADERS})

    ### coverage
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fexceptions")

    add_definitions(-DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    include_directories(${GTEST_INCLUDE_DIRS})

    ###
    if (COMMONS_BASE_ONLY)
        add_executable(VDCommons_Test ${COMMONS_BASE_FILES} ${TEST_BASE_SOURCES} ${TEST_BASE_HEADERS})
    else()
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ed25519_tests.txt ${CMAKE_CURRENT_BINARY_DIR}/ed25519_tests.txt COPYONLY)
        add_executable(VDCommons_Test ${COMMONS_BASE_FILES} ${COMMONS_NETWORK_FILES} ${COMMONS_CURVE_FILES} ${TEST_BASE_FILES} ${TEST_NETWORK_FILES} ${TEST_CURVE_FILES})
    endif()

    add_dependencies(VDCommons_Test protocol_generation enum_generation sqlite_generation)
    target_link_libraries(VDCommons_Test ${GTEST_BOTH_LIBRARIES} pthread VDCommons)

    ## code coverage
    include(CodeCoverage)
    if (LCOV_PATH AND NOT ANDROID)
        setup_target_for_coverage(VDCommons_Coverage VDCommons_Test ${CMAKE_CURRENT_BINARY_DIR}/coverage "'${CMAKE_CURRENT_SOURCE_DIR}/*';'${CMAKE_SOURCE_DIR}/external/*'")
    endif()

endif()