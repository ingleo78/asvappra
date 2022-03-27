#ifndef HEADER_CURL_TRANSFER_H
#define HEADER_CURL_TRANSFER_H

#define Curl_headersep(x) ((((x)==':') || ((x)==';')))
char *Curl_checkheaders(const struct connectdata *conn, const char *thisheader);
void Curl_init_CONNECT(struct Curl_easy *data);
CURLcode Curl_pretransfer(struct Curl_easy *data);
CURLcode Curl_posttransfer(struct Curl_easy *data);
typedef enum {
    FOLLOW_NONE,
    FOLLOW_FAKE,
    FOLLOW_RETRY,
    FOLLOW_REDIR,
    FOLLOW_LAST
} followtype;
CURLcode Curl_follow(struct Curl_easy *data, char *newurl, followtype type);
CURLcode Curl_readwrite(struct connectdata *conn, struct Curl_easy *data, bool *done, bool *comeback);
int Curl_single_getsock(const struct connectdata *conn, curl_socket_t *socks);
CURLcode Curl_readrewind(struct connectdata *conn);
CURLcode Curl_fillreadbuffer(struct connectdata *conn, size_t bytes, size_t *nreadp);
CURLcode Curl_retry_request(struct connectdata *conn, char **url);
bool Curl_meets_timecondition(struct Curl_easy *data, time_t timeofdoc);
CURLcode Curl_get_upload_buffer(struct Curl_easy *data);
CURLcode Curl_done_sending(struct connectdata *conn, struct SingleRequest *k);
void Curl_setup_transfer (struct Curl_easy *data, int sockindex, curl_off_t size, bool getheader, int writesockindex);

#endif