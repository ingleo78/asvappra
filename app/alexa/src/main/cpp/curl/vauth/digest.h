#ifndef HEADER_CURL_DIGEST_H
#define HEADER_CURL_DIGEST_H

#include "../curl.h"

#if !defined(CURL_DISABLE_CRYPTO_AUTH)
#define DIGEST_MAX_VALUE_LENGTH           256
#define DIGEST_MAX_CONTENT_LENGTH         1024
enum {
    CURLDIGESTALGO_MD5,
    CURLDIGESTALGO_MD5SESS,
    CURLDIGESTALGO_SHA256,
    CURLDIGESTALGO_SHA256SESS,
    CURLDIGESTALGO_SHA512_256,
    CURLDIGESTALGO_SHA512_256SESS
};
bool Curl_auth_digest_get_pair(const char *str, char *value, char *content, const char **endptr);
#endif

#endif