if (NOT TARGET ssl)
    add_subdirectory(openssl-cmake)
endif()
add_subdirectory(securememory)
