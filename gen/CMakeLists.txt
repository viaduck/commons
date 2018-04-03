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

# and targets
add_custom_target(auto_gen_files ALL DEPENDS ${GEN_HEADERS})

# fake target for linking and source files
add_library(auto_gen INTERFACE)
add_dependencies(auto_gen auto_gen_files)

# include autogen headers
target_include_directories(auto_gen INTERFACE ${GEN_INCLUDES})
