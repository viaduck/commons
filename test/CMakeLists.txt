set(TEST_FILES ${SOURCE_FILES} ${LIBCOM_HEADERS} ${CURVE25519_SOURCES} ConnectionTest.cpp ConnectionTest.h custom_assert.h BufferTest.cpp BufferTest.h ContainerTest.cpp ContainerTest.h
        BlockCacheTest.cpp BlockCacheTest.h StringTest.cpp StringTest.h DevNullTest.cpp ValidPtrTest.cpp ValidPtrTest.h
        KeyValueStorageTest.cpp KeyValueStorageTest.h SecureStorageTest.cpp SecureStorageTest.h ConversionTest.cpp
        ConversionTest.h UTF8DecoderTest.cpp UTF8DecoderTest.h BitfieldTest.cpp BitfieldTest.h
        X25519Test.cpp X25519Test.h ED25519Test.cpp ED25519Test.h file_test.cpp FastStringMapTest.cpp FastStringMapTest.h CertificateStorageTest.cpp CertificateStorageTest.h EnumTest.cpp EnumTest.h)

### coverage
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fexceptions")

add_definitions(-DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ed25519_tests.txt ${CMAKE_CURRENT_BINARY_DIR}/ed25519_tests.txt COPYONLY)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

###
add_executable(libCom_Test ${TEST_FILES})
add_dependencies(libCom_Test protocol_generation)
add_dependencies(libCom_Test enum_generation)
target_link_libraries(libCom_Test gtest_main)
target_link_libraries(libCom_Test libCom)

## code coverage
include(CodeCoverage)
if (LCOV_PATH AND NOT ANDROID)
    setup_target_for_coverage(libCom_Coverage libCom_Test ${CMAKE_CURRENT_BINARY_DIR}/coverage)
endif()
