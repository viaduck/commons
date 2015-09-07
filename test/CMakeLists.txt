set(TEST_FILES ${LIBCOM_HEADERS} BufferTest.cpp ContainerTest.cpp custom_assert.h)

###
add_executable(AllUnitTests ${TEST_FILES})
add_dependencies(AllUnitTests protocol_generation)
add_dependencies(AllUnitTests gtest_main)
add_dependencies(AllUnitTests libCom)
target_link_libraries(AllUnitTests gtest_main)
target_link_libraries(AllUnitTests libCom)

##
add_test(Tests AllUnitTests)
