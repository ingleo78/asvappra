#ifndef HEADER_CURL_SENDF_H
#define HEADER_CURL_SENDF_H

#include "curl_setup.h"

CURLcode Curl_sendf(curl_socket_t sockfd, struct connectdata *, const char *fmt, ...);
void Curl_infof(struct Curl_easy *, const char *fmt, ...);
void Curl_failf(struct Curl_easy *, const char *fmt, ...);
#if defined(CURL_DISABLE_VERBOSE_STRINGS)
#if defined(HAVE_VARIADIC_MACROS_C99)
#define infof(...)  Curl_nop_stmt
#elif defined(HAVE_VARIADIC_MACROS_GCC)
#define infof(x...)  Curl_nop_stmt
#else
#error "missing VARIADIC macro define, fix and rebuild!"
#endif
#else
#define infof Curl_infof
#endif
#define failf Curl_failf
#define CLIENTWRITE_BODY   (1<<0)
#define CLIENTWRITE_HEADER (1<<1)
#define CLIENTWRITE_BOTH   (CLIENTWRITE_BODY|CLIENTWRITE_HEADER)
CURLcode Curl_client_write(struct connectdata *conn, int type, char *ptr, size_t len) WARN_UNUSED_RESULT;
bool Curl_recv_has_postponed_data(struct connectdata *conn, int sockindex);
CURLcode Curl_read_plain(curl_socket_t sockfd, char *buf, size_t bytesfromsocket, ssize_t *n);
ssize_t Curl_recv_plain(struct connectdata *conn, int num, char *buf, size_t len, CURLcode *code);
ssize_t Curl_send_plain(struct connectdata *conn, int num, const void *mem, size_t len, CURLcode *code);
CURLcode Curl_read(struct connectdata *conn, curl_socket_t sockfd, char *buf, size_t buffersize, ssize_t *n);
CURLcode Curl_write(struct connectdata *conn, curl_socket_t sockfd, const void *mem, size_t len, ssize_t *written);
CURLcode Curl_write_plain(struct connectdata *conn, curl_socket_t sockfd, const void *mem, size_t len, ssize_t *written);
int Curl_debug(struct Curl_easy *data, curl_infotype type, char *ptr, size_t size);

#endif