#ifndef HEADER_CURL_WILDCARD_H
#define HEADER_CURL_WILDCARD_H

#include "curl_setup.h"
#ifndef CURL_DISABLE_FTP
#include "llist.h"

typedef enum {
    CURLWC_CLEAR = 0,
    CURLWC_INIT = 1,
    CURLWC_MATCHING,
    CURLWC_DOWNLOADING,
    CURLWC_CLEAN,
    CURLWC_SKIP,
    CURLWC_ERROR,
    CURLWC_DONE
} curl_wildcard_states;
typedef void (*curl_wildcard_dtor)(void *ptr);
struct WildcardData {
    curl_wildcard_states state;
    char *path;
    char *pattern;
    struct curl_llist filelist;
    void *protdata;
    curl_wildcard_dtor dtor;
    void *customptr;
};
CURLcode Curl_wildcard_init(struct WildcardData *wc);
void Curl_wildcard_dtor(struct WildcardData *wc);
struct Curl_easy;
#else
#define Curl_wildcard_dtor(x)
#endif
#endif