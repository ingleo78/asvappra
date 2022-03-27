#ifndef HEADER_CURL_MD4_H
#define HEADER_CURL_MD4_H

#include "curl_setup.h"

#if !defined(CURL_DISABLE_CRYPTO_AUTH)
#define MD4_DIGEST_LENGTH 16
void Curl_md4it(unsigned char *output, const unsigned char *input, const size_t len);
#endif
#endif