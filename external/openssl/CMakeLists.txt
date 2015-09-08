set(OPENSSL_BRANCH OpenSSL_1_0_2-stable)
set(NUM_JOBS 5)

set(OS "UNIX")

if (WIN32)
    # yep, windows need a special treatment, but neither cygwin nor msys, since they provide an UNIX-like environment
    if (MINGW)
        message(WARNING "Building on windows might fail")
        set(OS "WIN32")
        find_program(MSYS_BASH "bash.exe" PATHS "C:/Msys/" PATH_SUFFIXES "/1.0/bin/" "/bin/"
                DOC "Path to MSYS installation")
        if (NOT MSYS_BASH)
            message(FATAL_ERROR "Specify MSYS installation path")
        endif(NOT MSYS_BASH)
        message(WARNING "I assume your make program is a sibling of your compiler (resides in same directory)")
        set(MINGW_MAKE ${CMAKE_MAKE_PROGRAM})
    elseif(NOT (CYGWIN OR MSYS))
        message(FATAL_ERROR "Unsupported compiler infrastructure")
    endif(MINGW)
elseif(NOT UNIX)
    message(FATAL_ERROR "Unsupported platform, sorry")
endif()

# python helper script for corrent building environment
set(BUILD_ENV_TOOL ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/building_env.py ${OS} ${MSYS_BASH} ${MINGW_MAKE})

include(ExternalProject)
ExternalProject_Add(openssl
    GIT_REPOSITORY git://git.openssl.org/openssl.git
    GIT_TAG ${OPENSSL_BRANCH}

    CONFIGURE_COMMAND ${BUILD_ENV_TOOL} <SOURCE_DIR> ./config --prefix=${CMAKE_CURRENT_BINARY_DIR} no-cast no-md2 no-md4 no-mdc2 no-rc4 no-rc5 no-engines
        no-idea no-mdc2 no-rc5 no-camellia no-ripemd no-ssl2 no-ssl3 no-heartbeats no-gost no-sha0 no-deprecated no-capieng no-decc-init no-comp no-dtls no-psk no-srp
    # no-dsa no-rc2 no-des break tests, therefore we need them

    UPDATE_COMMAND ""

    BUILD_COMMAND ${BUILD_ENV_TOOL} <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} depend
        COMMAND ${BUILD_ENV_TOOL} <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} -j ${NUM_JOBS}

    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${BUILD_ENV_TOOL} <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} test

    INSTALL_COMMAND ${BUILD_ENV_TOOL} <SOURCE_DIR> ${CMAKE_MAKE_PROGRAM} install

    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_TEST 1
)

add_library(ssl STATIC IMPORTED GLOBAL)
add_library(crypto STATIC IMPORTED GLOBAL)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lib/libssl.a)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lib/libcrypto.a)
