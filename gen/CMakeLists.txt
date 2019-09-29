cmake_minimum_required(VERSION 3.1)

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
        set(GEN_FLATBUFFER_ARCH x86_64-windows)
    elseif (CMAKE_HOST_APPLE)
        set(GEN_FLATBUFFER_ARCH x86_64-osx)
    elseif (CMAKE_HOST_UNIX)
        set(GEN_FLATBUFFER_ARCH x86_64-linux)
    endif()
    
    if (NOT FLATC_VERSION)
        set(FLATC_VERSION "1.10.0")
    endif()

    # only download if needed
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz)
        set(PREBUILT_URL https://builds.viaduck.org/prebuilts/flatbuffers/${FLATC_VERSION}/${GEN_FLATBUFFER_ARCH}.tar.gz)
        file(DOWNLOAD ${PREBUILT_URL} ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar x ${CMAKE_CURRENT_BINARY_DIR}/flatc.tar.gz WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    set(FLATC_PROGRAM ${CMAKE_CURRENT_BINARY_DIR}/${GEN_FLATBUFFER_ARCH}/bin/flatc CACHE INTERNAL "" FORCE)
endif()

# output directory
set(GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

function(gen_add TARGET DEF_DIR)
    set(GEN_INCLUDES ${CMAKE_BINARY_DIR}/include)

    # get list of headers that need to be generated
    execute_process(COMMAND ${PYTHON_EXECUTABLE}
            ${GEN_DIR}/generator.py list ${DEF_DIR} ${GEN_INCLUDES}
            WORKING_DIRECTORY ${GEN_DIR}
            OUTPUT_VARIABLE GEN_HEADERS)

    # get list of source files to depend on
    execute_process(COMMAND ${PYTHON_EXECUTABLE}
            ${GEN_DIR}/generator.py depend ${DEF_DIR} ${GEN_INCLUDES}
            WORKING_DIRECTORY ${GEN_DIR}
            OUTPUT_VARIABLE GEN_DEPENDS)

    # custom command for actually generating the headers using the spec files
    add_custom_command(
            OUTPUT ${GEN_HEADERS}
            COMMAND ${PYTHON_EXECUTABLE}
            ${GEN_DIR}/generator.py generate ${DEF_DIR} ${GEN_INCLUDES} ${FLATC_PROGRAM}
            WORKING_DIRECTORY ${GEN_DIR}
            DEPENDS ${GEN_DEPENDS}
    )

    # add targets
    add_custom_target(${TARGET}_gen_files ALL DEPENDS ${GEN_HEADERS})

    # fake target for linking and source files
    add_library(${TARGET}_gen INTERFACE)
    add_dependencies(${TARGET}_gen Commons_gen_files)

    # include autogen headers
    target_include_directories(${TARGET}_gen INTERFACE ${GEN_INCLUDES} ${GEN_DIR}/../external/flatbuffers/include)
endfunction()
