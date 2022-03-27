#ifndef HEADER_CURL_COOKIE_H
#define HEADER_CURL_COOKIE_H

#include "curl_setup.h"
#include "curl.h"

struct Cookie {
    struct Cookie *next;
    char *name;
    char *value;
    char *path;
    char *spath;
    char *domain;
    curl_off_t expires;
    char *expirestr;
    bool tailmatch;
    char *version;
    char *maxage;
    bool secure;
    bool livecookie;
    bool httponly;
    int creationtime;
    unsigned char prefix;
};
#define COOKIE_PREFIX__SECURE (1<<0)
#define COOKIE_PREFIX__HOST (1<<1)
#define COOKIE_HASH_SIZE 256
struct CookieInfo {
    struct Cookie *cookies[COOKIE_HASH_SIZE];
    char *filename;
    bool running;
    long numcookies;
    bool newsession;
    int lastct;
};
#define MAX_COOKIE_LINE 5000
#define MAX_NAME 4096
#define MAX_NAME_TXT "4095"
struct Curl_easy;
struct Cookie *Curl_cookie_add(struct Curl_easy *data, struct CookieInfo *, bool header, bool noexpiry, char *lineptr, const char *domain, const char *path,
                               bool secure);
struct Cookie *Curl_cookie_getlist(struct CookieInfo *, const char *, const char *, bool);
void Curl_cookie_freelist(struct Cookie *cookies);
void Curl_cookie_clearall(struct CookieInfo *cookies);
void Curl_cookie_clearsess(struct CookieInfo *cookies);
#if defined(CURL_DISABLE_HTTP) || defined(CURL_DISABLE_COOKIES)
#define Curl_cookie_list(x) NULL
#define Curl_cookie_loadfiles(x) Curl_nop_stmt
#define Curl_cookie_init(x,y,z,w) NULL
#define Curl_cookie_cleanup(x) Curl_nop_stmt
#define Curl_flush_cookies(x,y) Curl_nop_stmt
#else
void Curl_flush_cookies(struct Curl_easy *data, bool cleanup);
void Curl_cookie_cleanup(struct CookieInfo *);
struct CookieInfo *Curl_cookie_init(struct Curl_easy *data, const char *, struct CookieInfo *, bool);
struct curl_slist *Curl_cookie_list(struct Curl_easy *data);
void Curl_cookie_loadfiles(struct Curl_easy *data);
#endif
#endif