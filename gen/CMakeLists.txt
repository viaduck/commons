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

## flatbuffers

if (SYSTEM_FLATC)
    # use system provided flatc
    find_program(FLATC_PROGRAM flatc)
    if (NOT FLATC_PROGRAM)
        message(FATAL_ERROR "Could not find flatc. Please install flatbuffers.")
    endif()
else()
    # download prebuilt flatc

    # detect host system
    if (CMAKE_HOST_WIN32)
        set(GEN_FLATBUFFER_ARCH win32)
    elseif (CMAKE_HOST_APPLE)
        set(GEN_FLATBUFFER_ARCH osx-x86_64)
    elseif (CMAKE_HOST_UNIX)
        set(GEN_FLATBUFFER_ARCH x86_64-linux)
    endif()

    # only download if needed
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz)
        set(PREBUILT_URL https://mirror.viaduck.org/prebuilts/flatbuffers/${GEN_FLATBUFFER_ARCH}.tar.gz)
        file(DOWNLOAD ${PREBUILT_URL} ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    set(FLATC_PROGRAM ${CMAKE_CURRENT_BINARY_DIR}/${GEN_FLATBUFFER_ARCH}/bin/flatc)
endif()

# custom command for actually generating the headers using the spec files
add_custom_command(
        OUTPUT ${GEN_HEADERS}
        COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/generator.py generate ${CMAKE_CURRENT_SOURCE_DIR} ${GEN_INCLUDES} ${FLATC_PROGRAM}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${GEN_DEPENDS}
)

# add targets
add_custom_target(Commons_gen_files ALL DEPENDS ${GEN_HEADERS})

# fake target for linking and source files
add_library(Commons_gen INTERFACE)
add_dependencies(Commons_gen Commons_gen_files)

# include autogen headers
target_include_directories(Commons_gen INTERFACE ${GEN_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/../external/flatbuffers/include)
