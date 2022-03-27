#ifndef HEADER_CURL_SSLUSE_H
#define HEADER_CURL_SSLUSE_H

#include "../curl_setup.h"
#ifdef USE_OPENSSL
#include "urldata.h"

extern const struct Curl_ssl Curl_ssl_openssl;
#endif
#endif