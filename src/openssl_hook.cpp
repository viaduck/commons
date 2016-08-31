#include "libCom/openssl_hook.h"
#include <openssl/ssl.h>

void global_initOpenSSL() {
    if (opensslInitialized)
        return;

#ifdef __WIN32
    WSADATA w;
	if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
		// TODO proper fatal shutdown
		return;
	}
#endif

    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);

    opensslInitialized = true;
}

void global_shutdownOpenSSL() {
#ifdef __WIN32
    WSACleanup();
#endif
}
