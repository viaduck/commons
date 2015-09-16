#ifndef LIBCOM_OPENSSL_HOOK_H
#define LIBCOM_OPENSSL_HOOK_H

static bool opensslInitiliazed = false;

void global_initOpenSSL();
void global_shutdownOpenSSL();

#endif //LIBCOM_OPENSSL_HOOK_H
