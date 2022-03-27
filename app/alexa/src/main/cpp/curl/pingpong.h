#ifndef HEADER_CURL_PINGPONG_H
#define HEADER_CURL_PINGPONG_H

#include <stdarg.h>
#include "curl_setup.h"

#if !defined(CURL_DISABLE_IMAP) || !defined(CURL_DISABLE_FTP) || !defined(CURL_DISABLE_POP3) || !defined(CURL_DISABLE_SMTP)
#define USE_PINGPONG
#endif
struct connectdata;
typedef enum {
    FTPTRANSFER_BODY,
    FTPTRANSFER_INFO,
    FTPTRANSFER_NONE,
    FTPTRANSFER_LAST
} curl_pp_transfer;
struct pingpong {
    char *cache;
    size_t cache_size;
    size_t nread_resp;
    char *linestart_resp;
    bool pending_resp;
    char *sendthis;
    size_t sendleft;
    size_t sendsize;
    struct curltime response;
    timediff_t response_time;
    struct connectdata *conn;
    CURLcode (*statemach_act)(struct connectdata *conn);
    bool (*endofresp)(struct connectdata *conn, char *ptr, size_t len, int *code);
};
CURLcode Curl_pp_statemach(struct pingpong *pp, bool block, bool disconnecting);
void Curl_pp_init(struct pingpong *pp);
timediff_t Curl_pp_state_timeout(struct pingpong *pp, bool disconnecting);
CURLcode Curl_pp_sendf(struct pingpong *pp, const char *fmt, ...);
CURLcode Curl_pp_vsendf(struct pingpong *pp, const char *fmt, va_list args);
CURLcode Curl_pp_readresp(curl_socket_t sockfd, struct pingpong *pp, int *code, size_t *size);
CURLcode Curl_pp_flushsend(struct pingpong *pp);
CURLcode Curl_pp_disconnect(struct pingpong *pp);
int Curl_pp_getsock(struct pingpong *pp, curl_socket_t *socks);
bool Curl_pp_moredata(struct pingpong *pp);

#endif