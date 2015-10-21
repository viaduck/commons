#ifndef LIBCOM_OPENSSL_HOOK_H
#define LIBCOM_OPENSSL_HOOK_H

#ifdef __WIN32
#include <winsock2.h>
#endif

/**
 *
 */
static bool opensslInitialized = false;     // FIXME this is ugly and of course not part of best practices

/**
 * Application wide OpenSSL initialization
 */
void global_initOpenSSL();

/**
 * Application wide OpenSSL shutdown
 */
void global_shutdownOpenSSL();

#endif //LIBCOM_OPENSSL_HOOK_H
