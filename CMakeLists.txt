cmake_minimum_required(VERSION 3.0)
project(VDCommons)

# add additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/cmake-modules")

# required packages
find_package(PythonInterp 3.4 REQUIRED)       # need the pathlib module, which is part of 3.4+

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
add_definitions(-DOPENSSL_API_COMPAT=0x10100000L)       # OpenSSL API compatibility

# externals
add_subdirectory(external)
include_directories(${CMAKE_BINARY_DIR}/include)

# configure openssl dirs
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/openssl-cmake/openssl-config.cmake")
    include(external/openssl-cmake/openssl-config.cmake)
    config_openssl(external/openssl-cmake)
endif()

# collect files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/commons/*.cpp)
file(GLOB_RECURSE VDCOMMONS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE CURVE25519_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/curve25519/*)

# targets
## get list of messages that must be generated
execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_PROTOCOL)
execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/enum/
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_ENUM)

## command that provides to be generated messages
file(GLOB_RECURSE THX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.thx)
add_custom_command(
    OUTPUT ${GENERATED_PROTOCOL}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/definition_generation.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/ ${CMAKE_BINARY_DIR} protocol
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/protocol.template.h ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_generator.py ${THX_FILES}
)

file(GLOB_RECURSE THE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/enum/*.the)
add_custom_command(
    OUTPUT ${GENERATED_ENUM}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/definition_generation.py ${CMAKE_CURRENT_SOURCE_DIR}/enum/ ${CMAKE_BINARY_DIR} enum
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/enum.template.h ${CMAKE_CURRENT_SOURCE_DIR}/tools/enum_generator.py ${THE_FILES}
)

## and the target
add_custom_target(protocol_generation ALL DEPENDS ${GENERATED_PROTOCOL})
add_custom_target(enum_generation ALL DEPENDS ${GENERATED_ENUM})

# build
add_library(VDCommons STATIC ${SOURCE_FILES} ${VDCOMMONS_HEADERS})
add_library(curve25519 STATIC ${CURVE25519_SOURCES})
add_dependencies(VDCommons protocol_generation)
add_dependencies(VDCommons enum_generation)
add_dependencies(VDCommons openssl)
add_dependencies(curve25519 openssl)
target_link_libraries(VDCommons ssl crypto curve25519 SecureMemory)
if (WIN32)
    target_link_libraries(VDCommons ws2_32)
endif()

# includes
target_include_directories(VDCommons PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/src/curve25519/
        )

target_include_directories(curve25519 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        )

# doxygen
include(Doxygen)
if (DOXYGEN_FOUND)
    setup_doxygen(VDCommons_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
endif()

# tests
add_subdirectory(test)
