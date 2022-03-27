#ifndef HEADER_CURL_SHARE_H
#define HEADER_CURL_SHARE_H

#include "curl_setup.h"
#include "curl.h"
#include "cookie.h"
#include "psl.h"
#include "urldata.h"
#include "conncache.h"

#ifdef __SALFORDC__
#define CURL_VOLATILE
#else
#define CURL_VOLATILE volatile
#endif
struct Curl_share {
    unsigned int specifier;
    CURL_VOLATILE unsigned int dirty;
    curl_lock_function lockfunc;
    curl_unlock_function unlockfunc;
    void *clientdata;
    struct conncache conn_cache;
    struct curl_hash hostcache;
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
    struct CookieInfo *cookies;
#endif
#ifdef USE_LIBPSL
    struct PslCache psl;
#endif
    struct curl_ssl_session *sslsession;
    size_t max_ssl_sessions;
    long sessionage;
};
CURLSHcode Curl_share_lock(struct Curl_easy *, curl_lock_data, curl_lock_access);
CURLSHcode Curl_share_unlock(struct Curl_easy *, curl_lock_data);

#endif