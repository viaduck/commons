cmake_minimum_required(VERSION 3.0)

project(libCom)

enable_testing()

# add additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/cmake-modules")

# required packages
find_package(PythonInterp 3 REQUIRED)       # use Python 3 - Python2 is not compatible!

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

# user settable settings
set(OPTION_SECURE_UNIQUEPTR ON CACHE BOOL "Erase memory on unique ptr deletion")
if (OPTION_SECURE_UNIQUEPTR)
    add_definitions(-DOPTION_SECURE_UNIQUEPTR)
endif()

# configure openssl dirs
if (NOT TARGET ssl)
    include(external/openssl-cmake/openssl-config.cmake)
    config_openssl(external/openssl-cmake)
endif()

# externals
add_subdirectory(external)
include(external/includes.cmake)
config_compiler_and_linker()

# includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/SecureUniquePtr.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Buffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/DevNull.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/Request.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/openssl_hook.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/String.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/KeyValueStorage.cpp) #

set(LIBCOM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Buffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Buffer.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/SecureUniquePtr.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/BlockCache.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Range.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/String.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/DevNull.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Request.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/openssl_hook.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/helper.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/conversions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/_conversions_crossplatform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/errorcodes.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/BufferRange.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/ValidPtr.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/KeyValueStorage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/Serializable.h ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/SecureStorage.h)

# targets
## get list of messages that must be generated
execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_PROTOCOL)

## command that provides to be generated messages
add_custom_command(
    OUTPUT ${GENERATED_PROTOCOL}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_generation.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/ ${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/libCom/message.template.h
        ${CMAKE_CURRENT_SOURCE_DIR}/protocol/database/*.thx ${CMAKE_CURRENT_SOURCE_DIR}/protocol/network/*.thx
        ${CMAKE_CURRENT_SOURCE_DIR}/protocol/push/*.thx ${CMAKE_CURRENT_SOURCE_DIR}/protocol/client/*.thx
        ${CMAKE_CURRENT_SOURCE_DIR}/protocol/test/*.thx
)

## and the target
add_custom_target(protocol_generation ALL DEPENDS ${GENERATED_PROTOCOL})

# build
add_library(libCom STATIC ${SOURCE_FILES} ${LIBCOM_HEADERS})
add_dependencies(libCom protocol_generation)
add_dependencies(libCom openssl)
target_link_libraries(libCom ssl crypto)
if (WIN32)
    target_link_libraries(libCom ws2_32)
endif()

# doxygen
include(Doxygen)
if (DOXYGEN_FOUND)
    setup_doxygen(libCom_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
endif()

# tests
add_subdirectory(test)
