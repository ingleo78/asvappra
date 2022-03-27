#ifndef HEADER_CURL_HTTP2_H
#define HEADER_CURL_HTTP2_H

#include "curl_setup.h"

#ifdef USE_NGHTTP2
#include "http.h"

#define DEFAULT_MAX_CONCURRENT_STREAMS 13
int Curl_http2_ver(char *p, size_t len);
const char *Curl_http2_strerror(uint32_t err);
CURLcode Curl_http2_init(struct connectdata *conn);
void Curl_http2_init_state(struct UrlState *state);
void Curl_http2_init_userset(struct UserDefined *set);
CURLcode Curl_http2_request_upgrade(struct dynbuf *req, struct connectdata *conn);
CURLcode Curl_http2_setup(struct connectdata *conn);
CURLcode Curl_http2_switched(struct connectdata *conn, const char *data, size_t nread);
void Curl_http2_setup_conn(struct connectdata *conn);
void Curl_http2_setup_req(struct Curl_easy *data);
void Curl_http2_done(struct Curl_easy *data, bool premature);
CURLcode Curl_http2_done_sending(struct connectdata *conn);
CURLcode Curl_http2_add_child(struct Curl_easy *parent, struct Curl_easy *child, bool exclusive);
void Curl_http2_remove_child(struct Curl_easy *parent, struct Curl_easy *child);
void Curl_http2_cleanup_dependencies(struct Curl_easy *data);
CURLcode Curl_http2_stream_pause(struct Curl_easy *data, bool pause);
bool Curl_h2_http_1_1_error(struct connectdata *conn);
#else
#define Curl_http2_request_upgrade(x,y) CURLE_UNSUPPORTED_PROTOCOL
#define Curl_http2_setup(x) CURLE_UNSUPPORTED_PROTOCOL
#define Curl_http2_switched(x,y,z) CURLE_UNSUPPORTED_PROTOCOL
#define Curl_http2_setup_conn(x) Curl_nop_stmt
#define Curl_http2_setup_req(x)
#define Curl_http2_init_state(x)
#define Curl_http2_init_userset(x)
#define Curl_http2_done(x,y)
#define Curl_http2_done_sending(x)
#define Curl_http2_add_child(x, y, z)
#define Curl_http2_remove_child(x, y)
#define Curl_http2_cleanup_dependencies(x)
#define Curl_http2_stream_pause(x, y)
#define Curl_h2_http_1_1_error(x) 0
#endif

#endif