#include "curl_setup.h"
#include "curl.h"

#ifdef USE_LIBPSL
#include "psl.h"
#include "share.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

void Curl_psl_destroy(struct PslCache *pslcache) {
    if(pslcache->psl) {
        if(pslcache->dynamic) psl_free((psl_ctx_t *) pslcache->psl);
        pslcache->psl = NULL;
        pslcache->dynamic = FALSE;
    }
}
static time_t now_seconds(void) {
    struct curltime now = Curl_now();
    return now.tv_sec;
}
const psl_ctx_t *Curl_psl_use(struct Curl_easy *easy) {
    struct PslCache *pslcache = easy->psl;
    const psl_ctx_t *psl;
    time_t now;
    if(!pslcache) return NULL;
    Curl_share_lock(easy, CURL_LOCK_DATA_PSL, CURL_LOCK_ACCESS_SHARED);
    now = now_seconds();
    if(!pslcache->psl || pslcache->expires <= now) {
        Curl_share_unlock(easy, CURL_LOCK_DATA_PSL);
        Curl_share_lock(easy, CURL_LOCK_DATA_PSL, CURL_LOCK_ACCESS_SINGLE);
        now = now_seconds();
        if(!pslcache->psl || pslcache->expires <= now) {
            bool dynamic = FALSE;
            time_t expires = TIME_T_MAX;
        #if defined(PSL_VERSION_NUMBER) && PSL_VERSION_NUMBER >= 0x001000
            psl = psl_latest(NULL);
            dynamic = psl != NULL;
            expires = now < TIME_T_MAX - PSL_TTL? now + PSL_TTL: TIME_T_MAX;
            if(!psl && !pslcache->dynamic)
        #endif
            psl = psl_builtin();
            if(psl) {
                Curl_psl_destroy(pslcache);
                pslcache->psl = psl;
                pslcache->dynamic = dynamic;
                pslcache->expires = expires;
            }
        }
        Curl_share_unlock(easy, CURL_LOCK_DATA_PSL);
        Curl_share_lock(easy, CURL_LOCK_DATA_PSL, CURL_LOCK_ACCESS_SHARED);
    }
    psl = pslcache->psl;
    if(!psl) Curl_share_unlock(easy, CURL_LOCK_DATA_PSL);
    return psl;
}
void Curl_psl_release(struct Curl_easy *easy) {
    Curl_share_unlock(easy, CURL_LOCK_DATA_PSL);
}
#endif