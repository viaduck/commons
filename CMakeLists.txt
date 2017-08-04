cmake_minimum_required(VERSION 3.0)

project(libCom)

enable_testing()

# add additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/cmake-modules")

# required packages
find_package(PythonInterp 3.4 REQUIRED)       # need the pathlib module, which is part of 3.4+

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
add_definitions(-DOPENSSL_API_COMPAT=0x10100000L)       # OpenSSL API compatibility

# user settable settings
set(OPTION_SECURE_UNIQUEPTR ON CACHE BOOL "Erase memory on unique ptr deletion")
if (OPTION_SECURE_UNIQUEPTR)
    add_definitions(-DOPTION_SECURE_UNIQUEPTR)
endif()

# externals
add_subdirectory(external)
include(external/includes.cmake)
config_compiler_and_linker()

# configure openssl dirs
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/openssl-cmake/openssl-config.cmake")
    include(external/openssl-cmake/openssl-config.cmake)
    config_openssl(external/openssl-cmake)
endif()

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE LIBCOM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

file(GLOB_RECURSE CURVE25519_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/curve25519/*)

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
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/protocol.template.h ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_generator.py ${THX_FILES}
)

file(GLOB_RECURSE THE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/enum/*.the)
add_custom_command(
    OUTPUT ${GENERATED_ENUM}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/definition_generation.py ${CMAKE_CURRENT_SOURCE_DIR}/enum/ ${CMAKE_BINARY_DIR} enum
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/enum.template.h ${CMAKE_CURRENT_SOURCE_DIR}/tools/enum_generator.py ${THE_FILES}
)

## and the target
add_custom_target(protocol_generation ALL DEPENDS ${GENERATED_PROTOCOL})
add_custom_target(enum_generation ALL DEPENDS ${GENERATED_ENUM})

# build
add_library(libCom STATIC ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/src/network/NativeWrapper.cpp ${LIBCOM_HEADERS})
add_library(curve25519 STATIC ${CURVE25519_SOURCES})
add_dependencies(libCom protocol_generation)
add_dependencies(libCom enum_generation)
add_dependencies(libCom openssl)
add_dependencies(libCom curve25519)
add_dependencies(curve25519 openssl)
target_link_libraries(libCom ssl crypto curve25519)
if (WIN32)
    target_link_libraries(libCom ws2_32)
endif()

# includes
target_include_directories(libCom PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/curve25519/
        )

# doxygen
include(Doxygen)
if (DOXYGEN_FOUND)
    setup_doxygen(libCom_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
endif()

# tests
add_subdirectory(test)
