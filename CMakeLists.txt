cmake_minimum_required(VERSION 2.8.4)

project(libCom)

enable_testing()


# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

# externals
add_subdirectory(external)
include(external/includes.cmake)
config_compiler_and_linker()

include(external/openssl-cmake/openssl-config.cmake)
config_openssl()

# includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_BINARY_DIR}/external/openssl-cmake/include/)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/SecureUniquePtr.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Buffer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/BufferRange.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/src/DevNull.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Request.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/openssl_hook.cpp) #
set(LIBCOM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Buffer.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/conversions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Buffer.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/SecureUniquePtr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/conversions_crossplatform.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/BlockCache.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/BufferRange.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/DevNull.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Request.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/openssl_hook.h) #

# targets
## get list of messages that must be generated
execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol.messages
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_PROTOCOL)

## command that provides to be generated messages
add_custom_command(
    OUTPUT ${GENERATED_PROTOCOL}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_generation.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol.messages ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol.messages ${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.thx
)

## and the target
add_custom_target(protocol_generation ALL DEPENDS ${GENERATED_PROTOCOL})

# build
add_library(libCom STATIC ${SOURCE_FILES} ${LIBCOM_HEADERS})
add_dependencies(libCom protocol_generation)
add_dependencies(libCom ssl crypto)
target_link_libraries(libCom ssl crypto)
if (WIN32)
    target_link_libraries(libCom ws2_32)
endif()

# tests
add_subdirectory(test)
