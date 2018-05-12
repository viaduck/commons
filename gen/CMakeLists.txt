cmake_minimum_required(VERSION 3.1)

# output directory
set(GEN_INCLUDES ${CMAKE_BINARY_DIR}/include)

# get list of headers that need to be generated
execute_process(COMMAND ${PYTHON_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/generator.py list ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GEN_HEADERS)

# get list of source files to depend on
execute_process(COMMAND ${PYTHON_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/generator.py depend ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GEN_DEPENDS)

# custom command for actually generating the headers using the spec files
add_custom_command(
        OUTPUT ${GEN_HEADERS}
        COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/generator.py generate ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GEN_DEPENDS}
)

# add target
add_custom_target(Commons_gen_files ALL DEPENDS ${GEN_HEADERS})

# fake target for linking and source files
add_library(Commons_gen INTERFACE)
add_dependencies(Commons_gen Commons_gen_files)

# include autogen headers
target_include_directories(Commons_gen INTERFACE ${GEN_INCLUDES})

## protobuf

if (SYSTEM_PROTOC)
    # use system provided protoc
    find_program(PROTOC_PROGRAM protoc)
    if (NOT PROTOC_PROGRAM)
        message(FATAL_ERROR "Could not find protoc. Please install protobuf")
    endif()
else()
    # download prebuilt protoc

    # detect host system
    if (CMAKE_HOST_WIN32)
        set(GEN_PROTOBUF_ARCH win32)
    elseif (CMAKE_HOST_APPLE)
        set(GEN_PROTOBUF_ARCH osx-x86_64)
    elseif (CMAKE_HOST_UNIX)
        set(GEN_PROTOBUF_ARCH linux-x86_64)
    endif()

    # only download if needed
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/protoc.zip)
        set(PREBUILT_URL https://github.com/google/protobuf/releases/download/v3.5.1/protoc-3.5.1-${GEN_PROTOBUF_ARCH}.zip)
        file(DOWNLOAD ${PREBUILT_URL} ${CMAKE_CURRENT_BINARY_DIR}/protoc.zip SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_BINARY_DIR}/protoc.zip WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    set(PROTOC_PROGRAM ${CMAKE_CURRENT_BINARY_DIR}/bin/protoc)
endif()

# get list of protobuf files that need to be generated
execute_process(COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/protobuf_generator.py list ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES} ${PROTOC_PROGRAM}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GEN_PROTOBUF_FILES)

# get list of proto files to depend on
execute_process(COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/protobuf_generator.py depend ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES} ${PROTOC_PROGRAM}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GEN_PROTOBUF_DEPENDS)

# custom command for actually generating the files using proto files
add_custom_command(
        OUTPUT ${GEN_PROTOBUF_FILES}
        COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/protobuf_generator.py generate ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES} ${PROTOC_PROGRAM}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GEN_PROTOBUF_DEPENDS} ${GEN_HEADERS}
)

# library for protobuf
add_library(Commons_protobuf STATIC ${GEN_PROTOBUF_FILES})
target_include_directories(Commons_protobuf PUBLIC ${GEN_INCLUDES})
target_link_libraries(Commons_protobuf libprotobuf-lite)

