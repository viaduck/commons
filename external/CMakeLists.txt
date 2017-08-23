add_subdirectory(securememory)

if (NOT TARGET ssl)
    add_subdirectory(openssl-cmake)
endif()
