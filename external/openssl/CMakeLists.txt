set(OPENSSL_BRANCH OpenSSL_1_0_2-stable)
set(NUM_JOBS 5)

set(OS "UNIX")

if (WIN32)
    message(WARNING "Building with mingw might fail")
    set(OS "WIN32")
elseif(NOT UNIX)
    message(FATAL_ERROR "Unsupported platform")
endif()

include(ExternalProject)
ExternalProject_Add(openssl
    GIT_REPOSITORY git://git.openssl.org/openssl.git
    GIT_TAG ${OPENSSL_BRANCH}

    CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/patch_configure.py ${OS} <SOURCE_DIR> --prefix=${CMAKE_CURRENT_BINARY_DIR} no-cast no-md2 no-md4 no-mdc2 no-rc4 no-rc5 no-engines
        no-idea no-mdc2 no-rc5 no-camellia no-ripemd no-ssl2 no-ssl3 no-heartbeats no-gost no-sha0 no-deprecated no-capieng no-decc-init no-comp no-dtls no-psk no-srp
    # no-dsa no-rc2 no-des break tests, therefore we need them

    UPDATE_COMMAND ""

    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C <SOURCE_DIR> depend COMMAND ${CMAKE_MAKE_PROGRAM} -C <SOURCE_DIR> -j ${NUM_JOBS}

    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_MAKE_PROGRAM} -C <SOURCE_DIR> test

    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} -C <SOURCE_DIR> install

    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_TEST 1
)

add_library(ssl STATIC IMPORTED GLOBAL)
add_library(crypto STATIC IMPORTED GLOBAL)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lib/libssl.a)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lib/libcrypto.a)
