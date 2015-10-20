#ifndef LIBCOM_OPENSSL_HOOK_H
#define LIBCOM_OPENSSL_HOOK_H

#ifdef __WIN32
#include <winsock2.h>
#endif

static bool opensslInitiliazed = false;

void global_initOpenSSL();
void global_shutdownOpenSSL();

#endif //LIBCOM_OPENSSL_HOOK_H
