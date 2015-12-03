set(EXT ${CMAKE_CURRENT_SOURCE_DIR}/external)

macro(config_compiler_and_linker)
    include_directories(${EXT}/googletest/include)
    include_directories(${CMAKE_BINARY_DIR}/include)
endmacro()
