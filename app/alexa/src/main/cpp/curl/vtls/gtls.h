#ifndef HEADER_CURL_GTLS_H
#define HEADER_CURL_GTLS_H

#include "../curl_setup.h"

#ifdef USE_GNUTLS
#include "urldata.h"
extern const struct Curl_ssl Curl_ssl_gnutls;
#endif
#endif