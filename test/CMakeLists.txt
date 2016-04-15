set(TEST_FILES ${SOURCE_FILES} ${LIBCOM_HEADERS} custom_assert.h BufferTest.cpp BufferTest.h ContainerTest.cpp ContainerTest.h
        BlockCacheTest.cpp BlockCacheTest.h StringTest.cpp StringTest.h DevNullTest.cpp ValidPtrTest.cpp ValidPtrTest.h
        KeyValueStorageTest.cpp KeyValueStorageTest.h SecureStorageTest.cpp SecureStorageTest.h ConversionTest.cpp ConversionTest.h)

### coverage
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")

###
add_executable(AllUnitTests ${TEST_FILES})
add_dependencies(AllUnitTests protocol_generation)
add_dependencies(AllUnitTests gtest_main)
add_dependencies(AllUnitTests libCom)
target_link_libraries(AllUnitTests gtest_main)
target_link_libraries(AllUnitTests libCom)

##
add_test(Tests AllUnitTests)

## code coverage
include(CodeCoverage)
if (LCOV_PATH)
    setup_target_for_coverage(Coverage AllUnitTests ${CMAKE_CURRENT_BINARY_DIR}/coverage)
endif()
