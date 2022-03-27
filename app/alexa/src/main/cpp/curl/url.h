#ifndef HEADER_CURL_URL_H
#define HEADER_CURL_URL_H

#include "curl_setup.h"

#define READBUFFER_SIZE CURL_MAX_WRITE_SIZE
#define READBUFFER_MAX  CURL_MAX_READ_SIZE
#define READBUFFER_MIN  1024
#define UPLOADBUFFER_DEFAULT 65536
#define UPLOADBUFFER_MAX (2*1024*1024)
#define UPLOADBUFFER_MIN CURL_MAX_WRITE_SIZE

CURLcode Curl_init_do(struct Curl_easy *data, struct connectdata *conn);
CURLcode Curl_open(struct Curl_easy **curl);
CURLcode Curl_init_userdefined(struct Curl_easy *data);
void Curl_freeset(struct Curl_easy *data);
CURLcode Curl_uc_to_curlcode(CURLUcode uc);
CURLcode Curl_close(struct Curl_easy **datap);
CURLcode Curl_connect(struct Curl_easy *, bool *async, bool *protocol_connect);
CURLcode Curl_disconnect(struct Curl_easy *data, struct connectdata *, bool dead_connection);
CURLcode Curl_setup_conn(struct connectdata *conn, bool *protocol_done);
void Curl_free_request_state(struct Curl_easy *data);
CURLcode Curl_parse_login_details(const char *login, const size_t len, char **userptr, char **passwdptr, char **optionsptr);
const struct Curl_handler *Curl_builtin_scheme(const char *scheme);
bool Curl_is_ASCII_name(const char *hostname);
CURLcode Curl_idnconvert_hostname(struct connectdata *conn, struct hostname *host);
void Curl_free_idnconverted_hostname(struct hostname *host);

#define CURL_DEFAULT_PROXY_PORT 1080
#define CURL_DEFAULT_HTTPS_PROXY_PORT 443

#ifdef CURL_DISABLE_VERBOSE_STRINGS
#define Curl_verboseconnect(x)  Curl_nop_stmt
#else
void Curl_verboseconnect(struct connectdata *conn);
#endif

#ifdef CURL_DISABLE_PROXY
#define CONNECT_PROXY_SSL() FALSE
#else

#define CONNECT_PROXY_SSL() (conn->http_proxy.proxytype == CURLPROXY_HTTPS && !conn->bits.proxy_ssl_connected[sockindex])
#define CONNECT_FIRSTSOCKET_PROXY_SSL() (conn->http_proxy.proxytype == CURLPROXY_HTTPS && !conn->bits.proxy_ssl_connected[FIRSTSOCKET])
#define CONNECT_SECONDARYSOCKET_PROXY_SSL() (conn->http_proxy.proxytype == CURLPROXY_HTTPS && !conn->bits.proxy_ssl_connected[SECONDARYSOCKET])
#endif
#endif