#ifndef HEADER_CURL_ALTSVC_H
#define HEADER_CURL_ALTSVC_H

#include "curl_setup.h"

#if !defined(CURL_DISABLE_HTTP) && !defined(USE_ALTSVC)
#include "curl.h"
#include "llist.h"

enum alpnid {
    ALPN_none = 0,
    ALPN_h1 = CURLALTSVC_H1,
    ALPN_h2 = CURLALTSVC_H2,
    ALPN_h3 = CURLALTSVC_H3
};
struct althost {
    char *host;
    unsigned short port;
    enum alpnid alpnid;
};
struct altsvc {
    struct althost src;
    struct althost dst;
    time_t expires;
    bool persist;
    int prio;
    struct curl_llist_element node;
};
struct altsvcinfo {
    char *filename;
    struct curl_llist list;
    long flags;
};
const char *Curl_alpnid2str(enum alpnid id);
struct altsvcinfo *Curl_altsvc_init(void);
CURLcode Curl_altsvc_load(struct altsvcinfo *asi, const char *file);
CURLcode Curl_altsvc_save(struct Curl_easy *data, struct altsvcinfo *asi, const char *file);
CURLcode Curl_altsvc_ctrl(struct altsvcinfo *asi, const long ctrl);
void Curl_altsvc_cleanup(struct altsvcinfo *altsvc);
CURLcode Curl_altsvc_parse(struct Curl_easy *data, struct altsvcinfo *altsvc, const char *value, enum alpnid srcalpn, const char *srchost,
                           unsigned short srcport);
bool Curl_altsvc_lookup(struct altsvcinfo *asi, enum alpnid srcalpnid, const char *srchost, int srcport, struct altsvc **dstentry, const int versions);
#else
#define Curl_altsvc_save(a,b,c)
#endif
#endif