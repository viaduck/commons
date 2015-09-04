cmake_minimum_required(VERSION 2.8.4)

project(pushClient)

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

## externals
add_subdirectory(external)
include(external/includes.cmake)
config_compiler_and_linker()

# includes
include_directories(include/)

set(SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/SecureUniquePtr.cpp ${CMAKE_SOURCE_DIR}/src/Buffer.cpp ${CMAKE_SOURCE_DIR}/src/Socket.cpp)

# targets
add_executable(pushClient ${SOURCE_FILES})

## tests
add_subdirectory(test)


## building
add_dependencies(pushClient gtest_main)
target_link_libraries(pushClient gtest_main)


