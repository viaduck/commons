#include <openssl/evp.h>
#include <openssl/ssl.h>

#include "libCom/openssl_hook.h"

void global_initOpenSSL() {
    if (opensslInitiliazed)
        return;

#ifdef __WIN32
    WSADATA w;
	if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
		// TODO proper fatal shutdown
		return;
	}
#endif

#ifndef LIBCOM_OPENSSL_HOOK_NO_READD_ALGOS
    OpenSSL_add_all_algorithms();
#endif

    SSL_library_init();
    SSL_load_error_strings();

    opensslInitiliazed = true;
}

void global_shutdownOpenSSL() {
#ifdef __WIN32
    WSACleanup();
#endif
}
