#ifndef HEADER_CURL_SOCKS_H
#define HEADER_CURL_SOCKS_H

#include "curl_setup.h"

#ifdef CURL_DISABLE_PROXY
#define Curl_SOCKS4(a,b,c,d,e) CURLE_NOT_BUILT_IN
#define Curl_SOCKS5(a,b,c,d,e,f) CURLE_NOT_BUILT_IN
#define Curl_SOCKS_getsock(x,y,z) 0
#else
int Curl_blockread_all(struct connectdata *conn, curl_socket_t sockfd, char *buf, ssize_t buffersize, ssize_t *n);
int Curl_SOCKS_getsock(struct connectdata *conn, curl_socket_t *sock, int sockindex);
CURLcode Curl_SOCKS4(const char *proxy_name, const char *hostname, int remote_port, int sockindex, struct connectdata *conn, bool *done);
CURLcode Curl_SOCKS5(const char *proxy_name, const char *proxy_password, const char *hostname, int remote_port, int sockindex, struct connectdata *conn, bool *done);
#if defined(HAVE_GSSAPI) || defined(USE_WINDOWS_SSPI)
CURLcode Curl_SOCKS5_gssapi_negotiate(int sockindex, struct connectdata *conn);
#endif
#endif
#endif