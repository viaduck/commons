set(TEST_FILES ${SOURCE_FILES} BufferTest.cpp ContainerTest.cpp custom_assert.h)

###
add_executable(AllUnitTests ${TEST_FILES})
add_dependencies(AllUnitTests gtest_main)
target_link_libraries(AllUnitTests gtest_main)
