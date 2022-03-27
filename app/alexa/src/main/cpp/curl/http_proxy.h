#ifndef HEADER_CURL_HTTP_PROXY_H
#define HEADER_CURL_HTTP_PROXY_H

#include "curl_setup.h"
#include "urldata.h"

#if !defined(CURL_DISABLE_PROXY) && !defined(CURL_DISABLE_HTTP)
CURLcode Curl_proxyCONNECT(struct connectdata *conn, int tunnelsocket, const char *hostname, int remote_port);
CURLcode Curl_proxy_connect(struct connectdata *conn, int sockindex);
bool Curl_connect_complete(struct connectdata *conn);
bool Curl_connect_ongoing(struct connectdata *conn);
#else
#define Curl_proxyCONNECT(x,y,z,w) CURLE_NOT_BUILT_IN
#define Curl_proxy_connect(x,y) CURLE_OK
#define Curl_connect_complete(x) CURLE_OK
#define Curl_connect_ongoing(x) FALSE
#endif
void Curl_connect_free(struct Curl_easy *data);
void Curl_connect_done(struct Curl_easy *data);
#endif