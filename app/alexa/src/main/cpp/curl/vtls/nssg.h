#ifndef HEADER_CURL_NSSG_H
#define HEADER_CURL_NSSG_H

#include "../curl_setup.h"

#ifdef USE_NSS
#include "urldata.h"

CURLcode Curl_nss_force_init(struct Curl_easy *data);
extern const struct Curl_ssl Curl_ssl_nss;
#endif
#endif