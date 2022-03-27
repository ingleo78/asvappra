#ifndef HEADER_PSL_H
#define HEADER_PSL_H

#ifdef USE_LIBPSL
#include <libpsl.h>

#define PSL_TTL (72 * 3600)
struct PslCache {
    const psl_ctx_t *psl;
    time_t expires;
    bool dynamic;
};
const psl_ctx_t *Curl_psl_use(struct Curl_easy *easy);
void Curl_psl_release(struct Curl_easy *easy);
void Curl_psl_destroy(struct PslCache *pslcache);
#else
#define Curl_psl_use(easy) NULL
#define Curl_psl_release(easy)
#define Curl_psl_destroy(pslcache)
#endif
#endif