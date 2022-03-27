#ifndef HEADER_CURL_AMIGAOS_H
#define HEADER_CURL_AMIGAOS_H

#include "curl_setup.h"

#if defined(__AMIGA__) && defined(HAVE_BSDSOCKET_H) && !defined(USE_AMISSL)
bool Curl_amiga_init();
void Curl_amiga_cleanup();
#else
#define Curl_amiga_init() 1
#define Curl_amiga_cleanup() Curl_nop_stmt
#endif
#ifdef USE_AMISSL
#include <openssl/x509v3.h>
void Curl_amiga_X509_free(X509 *a);
#endif
#endif