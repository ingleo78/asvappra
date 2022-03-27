#include "curl_setup.h"
#include "curl.h"
#include "urldata.h"
#include "share.h"
#include "psl.h"
#include "vtls/vtls.h"
#include "curl_memory.h"
#include "memdebug.h"

CURLSH *curl_share_init(void) {
    struct Curl_share *share = calloc(1, sizeof(struct Curl_share));
    if(share) {
        share->specifier |= (1<<CURL_LOCK_DATA_SHARE);
        if(Curl_mk_dnscache(&share->hostcache)) {
            free(share);
            return NULL;
        }
    }
    return share;
}
#undef curl_share_setopt
CURLSHcode curl_share_setopt(CURLSH *share, CURLSHoption option, ...) {
    va_list param;
    int type;
    curl_lock_function lockfunc;
    curl_unlock_function unlockfunc;
    void *ptr;
    CURLSHcode res = CURLSHE_OK;
    if(((struct Curl_share*)share)->dirty) return CURLSHE_IN_USE;
    va_start(param, option);
    switch(option) {
        case CURLSHOPT_SHARE:
            type = va_arg(param, int);
            switch(type) {
                case CURL_LOCK_DATA_DNS: break;
                case CURL_LOCK_DATA_COOKIE:
                #if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
                    if(!((struct Curl_share*)share)->cookies) {
                        ((struct Curl_share*)share)->cookies = Curl_cookie_init(NULL, NULL, NULL, TRUE);
                        if(!((struct Curl_share*)share)->cookies) res = CURLSHE_NOMEM;
                    }
                #else
                    res = CURLSHE_NOT_BUILT_IN;
                #endif
                    break;
                case CURL_LOCK_DATA_SSL_SESSION:
                #ifdef USE_SSL
                    if(!share->sslsession) {
                        share->max_ssl_sessions = 8;
                        share->sslsession = calloc(share->max_ssl_sessions, sizeof(struct curl_ssl_session));
                        share->sessionage = 0;
                        if(!share->sslsession) res = CURLSHE_NOMEM;
                    }
                #else
                    res = CURLSHE_NOT_BUILT_IN;
                #endif
                    break;
                case CURL_LOCK_DATA_CONNECT:
                    if(Curl_conncache_init(&((struct Curl_share*)share)->conn_cache, 103)) res = CURLSHE_NOMEM;
                    break;
                case CURL_LOCK_DATA_PSL:
                #ifndef USE_LIBPSL
                    res = CURLSHE_NOT_BUILT_IN;
                #endif
                    break;
                default: res = CURLSHE_BAD_OPTION;
            }
            if(!res) ((struct Curl_share*)share)->specifier |= (1<<type);
            break;
        case CURLSHOPT_UNSHARE:
            type = va_arg(param, int);
            ((struct Curl_share*)share)->specifier &= ~(1<<type);
            switch(type) {
                case CURL_LOCK_DATA_DNS: break;
                case CURL_LOCK_DATA_COOKIE:
                #if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
                    if(((struct Curl_share*)share)->cookies) {
                        Curl_cookie_cleanup(((struct Curl_share*)share)->cookies);
                        ((struct Curl_share*)share)->cookies = NULL;
                    }
                #else
                    res = CURLSHE_NOT_BUILT_IN;
                #endif
                    break;
                case CURL_LOCK_DATA_SSL_SESSION:
                #ifdef USE_SSL
                    Curl_safefree(share->sslsession);
                #else
                    res = CURLSHE_NOT_BUILT_IN;
                #endif
                    break;
                case CURL_LOCK_DATA_CONNECT: break;
                default: res = CURLSHE_BAD_OPTION;
            }
            break;
        case CURLSHOPT_LOCKFUNC:
            lockfunc = va_arg(param, curl_lock_function);
            ((struct Curl_share*)share)->lockfunc = lockfunc;
            break;
        case CURLSHOPT_UNLOCKFUNC:
            unlockfunc = va_arg(param, curl_unlock_function);
            ((struct Curl_share*)share)->unlockfunc = unlockfunc;
            break;
        case CURLSHOPT_USERDATA:
            ptr = va_arg(param, void *);
            ((struct Curl_share*)share)->clientdata = ptr;
            break;
        default: res = CURLSHE_BAD_OPTION;
    }
    va_end(param);
    return res;
}
CURLSHcode curl_share_cleanup(CURLSH *share) {
    if(share == NULL) return CURLSHE_INVALID;
    if(((struct Curl_share*)share)->lockfunc) {
        ((struct Curl_share*)share)->lockfunc(NULL, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE, ((struct Curl_share*)share)->clientdata);
    }
    if(((struct Curl_share*)share)->dirty) {
        if(((struct Curl_share*)share)->unlockfunc) ((struct Curl_share*)share)->unlockfunc(NULL, CURL_LOCK_DATA_SHARE, ((struct Curl_share*)share)->clientdata);
        return CURLSHE_IN_USE;
    }
    Curl_conncache_close_all_connections(&((struct Curl_share*)share)->conn_cache);
    Curl_conncache_destroy(&((struct Curl_share*)share)->conn_cache);
    Curl_hash_destroy(&((struct Curl_share*)share)->hostcache);
#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
    Curl_cookie_cleanup(((struct Curl_share*)share)->cookies);
#endif
#ifdef USE_SSL
    if(share->sslsession) {
        size_t i;
        for(i = 0; i < ((struct Curl_share*)share)->max_ssl_sessions; i++) Curl_ssl_kill_session(&(((struct Curl_share*)share)->sslsession[i]));
        free(((struct Curl_share*)share)->sslsession);
    }
#endif
    Curl_psl_destroy(&((struct Curl_share*)share)->psl);
    if(((struct Curl_share*)share)->unlockfunc) ((struct Curl_share*)share)->unlockfunc(NULL, CURL_LOCK_DATA_SHARE, ((struct Curl_share*)share)->clientdata);
    free(share);
    return CURLSHE_OK;
}
CURLSHcode Curl_share_lock(struct Curl_easy *data, curl_lock_data type, curl_lock_access accesstype) {
    struct Curl_share *share = data->share;
    if(share == NULL) return CURLSHE_INVALID;
    if(share->specifier & (1<<type)) {
        if(share->lockfunc) share->lockfunc(data, type, accesstype, share->clientdata);
    }
    return CURLSHE_OK;
}
CURLSHcode Curl_share_unlock(struct Curl_easy *data, curl_lock_data type) {
    struct Curl_share *share = data->share;
    if(share == NULL) return CURLSHE_INVALID;
    if(share->specifier & (1<<type)) {
        if(share->unlockfunc) share->unlockfunc (data, type, share->clientdata);
    }
    return CURLSHE_OK;
}