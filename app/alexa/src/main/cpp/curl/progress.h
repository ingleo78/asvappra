#ifndef HEADER_CURL_PROGRESS_H
#define HEADER_CURL_PROGRESS_H

#include "timeval.h"

typedef enum {
    TIMER_NONE,
    TIMER_STARTOP,
    TIMER_STARTSINGLE,
    TIMER_NAMELOOKUP,
    TIMER_CONNECT,
    TIMER_APPCONNECT,
    TIMER_PRETRANSFER,
    TIMER_STARTTRANSFER,
    TIMER_POSTRANSFER,
    TIMER_STARTACCEPT,
    TIMER_REDIRECT,
    TIMER_LAST
} timerid;
int Curl_pgrsDone(struct connectdata *);
void Curl_pgrsStartNow(struct Curl_easy *data);
void Curl_pgrsSetDownloadSize(struct Curl_easy *data, curl_off_t size);
void Curl_pgrsSetUploadSize(struct Curl_easy *data, curl_off_t size);
void Curl_pgrsSetDownloadCounter(struct Curl_easy *data, curl_off_t size);
void Curl_pgrsSetUploadCounter(struct Curl_easy *data, curl_off_t size);
void Curl_ratelimit(struct Curl_easy *data, struct curltime now);
int Curl_pgrsUpdate(struct connectdata *);
void Curl_pgrsResetTransferSizes(struct Curl_easy *data);
void Curl_pgrsTime(struct Curl_easy *data, timerid timer);
timediff_t Curl_pgrsLimitWaitTime(curl_off_t cursize, curl_off_t startsize, curl_off_t limit, struct curltime start, struct curltime now);
#define PGRS_HIDE    (1<<4)
#define PGRS_UL_SIZE_KNOWN (1<<5)
#define PGRS_DL_SIZE_KNOWN (1<<6)
#define PGRS_HEADERS_OUT (1<<7)
#endif