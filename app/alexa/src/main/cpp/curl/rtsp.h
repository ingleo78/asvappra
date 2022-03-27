#ifndef HEADER_CURL_RTSP_H
#define HEADER_CURL_RTSP_H

#ifndef CURL_DISABLE_RTSP

typedef int ssize_t;
extern const struct Curl_handler Curl_handler_rtsp;

CURLcode Curl_rtsp_parseheader(struct connectdata *conn, char *header);
static CURLcode rtsp_do(struct connectdata *conn, bool *done);
static CURLcode rtsp_done(struct connectdata *conn, CURLcode, bool premature);
static CURLcode rtsp_connect(struct connectdata *conn, bool *done);
static CURLcode rtsp_disconnect(struct connectdata *conn, bool dead);
static int rtsp_getsock_do(struct connectdata *conn, curl_socket_t *socks);
static CURLcode rtsp_rtp_readwrite(struct Curl_easy *data, struct connectdata *conn, ssize_t *nread, bool *readmore);
static CURLcode rtsp_setup_connection(struct connectdata *conn);
static unsigned int rtsp_conncheck(struct connectdata *check, unsigned int checks_to_perform);
#else
#define Curl_rtsp_parseheader(x,y) CURLE_NOT_BUILT_IN
#endif
struct rtsp_conn {
    char *rtp_buf;
    ssize_t rtp_bufsize;
    int rtp_channel;
};
struct RTSP {
    struct HTTP http_wrapper;
    long CSeq_sent;
    long CSeq_recv;
};

#endif