#ifndef HEADER_CURL_HTTP_DIGEST_H
#define HEADER_CURL_HTTP_DIGEST_H

#include "curl_setup.h"

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_CRYPTO_AUTH)
CURLcode Curl_input_digest(struct connectdata *conn, bool proxy, const char *header);
CURLcode Curl_output_digest(struct connectdata *conn, bool proxy, const unsigned char *request, const unsigned char *uripath);
void Curl_http_auth_cleanup_digest(struct Curl_easy *data);
#endif
#endif