cmake_minimum_required(VERSION 2.8.4)

project(libCom)

enable_testing()


# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# externals
add_subdirectory(external)
include(external/includes.cmake)
config_compiler_and_linker()

# includes
include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_BINARY_DIR}/include/)

set(SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/SecureUniquePtr.cpp ${CMAKE_SOURCE_DIR}/src/Buffer.cpp)
set(LIBCOM_HEADERS ${CMAKE_SOURCE_DIR}/include/Buffer.h ${CMAKE_SOURCE_DIR}/include/conversions.h
    ${CMAKE_SOURCE_DIR}/include/Buffer.h ${CMAKE_SOURCE_DIR}/include/SecureUniquePtr.h)

# targets
## get list of messages that must be generated
execute_process(COMMAND python ${CMAKE_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_SOURCE_DIR}/protocol/protocol.messages
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_PROTOCOL)

## command that provides to be generated messages
add_custom_command(
    OUTPUT ${GENERATED_PROTOCOL}
    COMMAND python ${CMAKE_SOURCE_DIR}/tools/protocol_generation.py ${CMAKE_SOURCE_DIR}/protocol/protocol.messages ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/protocol/protocol.messages
)

## and the target
add_custom_target(protocol_generation ALL DEPENDS ${GENERATED_PROTOCOL})

# build
add_library(libCom ${SOURCE_FILES} ${LIBCOM_HEADERS})
add_dependencies(libCom protocol_generation)

# tests
add_subdirectory(test)
