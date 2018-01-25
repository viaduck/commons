cmake_minimum_required(VERSION 3.0)
project(VDCommons)

# add additional cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/securememory/cmake-modules")

# enable and require c++14 support
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# this variable allows turning off all network, ssl, and curve dependencies
set(COMMONS_BASE_ONLY OFF CACHE STRING "Turn off all dependencies of VDCommons and build only the basics")

# externals
add_subdirectory(external/securememory)

# collect files
file(GLOB_RECURSE COMMONS_BASE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/commons/*.cpp)
file(GLOB_RECURSE COMMONS_BASE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/*.h)
file(GLOB_RECURSE COMMONS_NETWORK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/network/*.cpp)
file(GLOB_RECURSE COMMONS_NETWORK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/network/*.h)
file(GLOB_RECURSE COMMONS_CURVE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/curve25519/*.cpp)
file(GLOB_RECURSE COMMONS_CURVE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/curve25519/*.h)

# convenience
set(COMMONS_BASIC_FILES ${COMMONS_BASE_SOURCES} ${COMMONS_BASE_HEADERS})
set(COMMONS_NETWORK_FILES ${COMMONS_NETWORK_SOURCES} ${COMMONS_NETWORK_HEADERS})
set(COMMONS_CURVE_FILES ${COMMONS_CURVE_SOURCES} ${COMMONS_CURVE_HEADERS})

if (COMMONS_BASE_ONLY)

    # create the library using only base files
    add_library(VDCommons STATIC ${COMMONS_BASIC_FILES})

else()

    # required packages
    # need the pathlib module, which is part of 3.4+
    find_package(PythonInterp 3.4 REQUIRED)

    # require openssl
    add_definitions(-DOPENSSL_API_COMPAT=0x10100000L)       # OpenSSL API compatibility
    if (NOT TARGET ssl)
        add_subdirectory(external/openssl-cmake)
    endif()

    # include autogen headers
    include_directories(${CMAKE_BINARY_DIR}/include)

    # targets
    ## get lists of headers that need to be generated
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/protocol/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_PROTOCOL)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/enum/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_ENUM)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/protocol_list.py ${CMAKE_CURRENT_SOURCE_DIR}/sqx/
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR} OUTPUT_VARIABLE GENERATED_SQX)

    ## custom command for actually generating the headers using the spec files
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

    file(GLOB_RECURSE SQX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sqx/*.sqx)
    add_custom_command(
        OUTPUT ${GENERATED_SQX}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/definition_generation.py ${CMAKE_CURRENT_SOURCE_DIR}/sqx/ ${CMAKE_BINARY_DIR} sqx
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/commons/enum.template.h ${CMAKE_CURRENT_SOURCE_DIR}/tools/enum_generator.py ${SQX_FILES}
    )

    ## and targets
    add_custom_target(protocol_generation ALL DEPENDS ${GENERATED_PROTOCOL})
    add_custom_target(enum_generation ALL DEPENDS ${GENERATED_ENUM})
    add_custom_target(sqlite_generation ALL DEPENDS ${GENERATED_SQX})

    # build
    add_library(VDCommons STATIC ${COMMONS_BASIC_FILES} ${COMMONS_NETWORK_FILES})
    add_library(curve25519 STATIC ${COMMONS_CURVE_FILES})

    # force a rebuild of commons if one of the spec files changes
    add_dependencies(VDCommons protocol_generation)
    add_dependencies(VDCommons enum_generation)
    add_dependencies(VDCommons sqlite_generation)

    # link to everything but the basics
    target_link_libraries(curve25519 ssl crypto)
    target_link_libraries(VDCommons ssl crypto curve25519)

    # ntoh related stuff on windows needs winsock
    if (WIN32)
        target_link_libraries(VDCommons ws2_32)
    endif()

    # export all includes but the basics
    target_include_directories(VDCommons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/curve25519/)
    target_include_directories(curve25519 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

    # doxygen
    include(Doxygen)
    if (DOXYGEN_FOUND)
        setup_doxygen(VDCommons_doc ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    endif()

    # tests
    add_subdirectory(test)

endif()

# link to base dependencies in any case
target_link_libraries(VDCommons SecureMemory)

# export the basic includes and compile options in any case
target_include_directories(VDCommons PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_options(VDCommons PUBLIC -Wall -Wextra)