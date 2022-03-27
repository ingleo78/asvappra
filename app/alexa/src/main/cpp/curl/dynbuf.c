#include <assert.h>
#include "dynbuf.h"
#include "curl_setup.h"
#include "strdup.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define MIN_FIRST_ALLOC 32
#define DYNINIT 0xbee51da
#define DEBUGASSERT(x) assert(x)

void Curl_dyn_init(struct dynbuf *s, size_t toobig) {
    DEBUGASSERT(s);
    DEBUGASSERT(toobig);
    s->bufr = NULL;
    s->leng = 0;
    s->allc = 0;
    s->toobig = toobig;
#ifdef DEBUGBUILD
    s->init = DYNINIT;
#endif
}
void Curl_dyn_free(struct dynbuf *s) {
    DEBUGASSERT(s);
    Curl_safefree(s->bufr);
    s->leng = s->allc = 0;
}
static CURLcode dyn_nappend(struct dynbuf *s, const unsigned char *mem, size_t len) {
    size_t indx = s->leng;
    size_t a = s->allc;
    size_t fit = len + indx + 1;
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(s->toobig);
    DEBUGASSERT(indx < s->toobig);
    DEBUGASSERT(!s->leng || s->bufr);
    if (fit > s->toobig) {
        Curl_dyn_free(s);
        return CURLE_OUT_OF_MEMORY;
    } else if (!a) {
        DEBUGASSERT(!indx);
        if(fit < MIN_FIRST_ALLOC) a = MIN_FIRST_ALLOC;
        else a = fit;
    } else while(a < fit) a *= 2;
    if (a != s->allc) {
        s->bufr = Curl_saferealloc(s->bufr, a);
        if (!s->bufr) {
          s->leng = s->allc = 0;
          return CURLE_OUT_OF_MEMORY;
        }
        s->allc = a;
    }
    if (len) memcpy(&s->bufr[indx], mem, len);
    s->leng = indx + len;
    s->bufr[s->leng] = 0;
    return CURLE_OK;
}
void Curl_dyn_reset(struct dynbuf *s) {
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    if (s->leng) s->bufr[0] = 0;
    s->leng = 0;
}
#ifdef USE_NGTCP2
CURLcode Curl_dyn_tail(struct dynbuf *s, size_t trail) {
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    if(trail > s->leng) return CURLE_BAD_FUNCTION_ARGUMENT;
    else if(trail == s->leng) return CURLE_OK;
    else if(!trail) Curl_dyn_reset(s);
    else {
        memmove(&s->bufr[0], &s->bufr[s->leng - trail], trail);
        s->leng = trail;
    }
    return CURLE_OK;
}
#endif
CURLcode Curl_dyn_addn(struct dynbuf *s, const void *mem, size_t len) {
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    return dyn_nappend(s, mem, len);
}
CURLcode Curl_dyn_add(struct dynbuf *s, const char *str) {
    size_t n = strlen(str);
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    return dyn_nappend(s, (unsigned char *)str, n);
}
CURLcode Curl_dyn_addf(struct dynbuf *s, const char *fmt, ...) {
    char *str;
    va_list ap;
    va_start(ap, fmt);
    str = vaprintf(fmt, ap);
    va_end(ap);
    if(str) {
        CURLcode result = dyn_nappend(s, (unsigned char *)str, strlen(str));
        free(str);
        return result;
    }
    Curl_dyn_free(s);
    return CURLE_OUT_OF_MEMORY;
}
char *Curl_dyn_ptr(const struct dynbuf *s) {
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    return s->bufr;
}
unsigned char *Curl_dyn_uptr(const struct dynbuf *s) {
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    return (unsigned char *)s->bufr;
}
size_t Curl_dyn_len(const struct dynbuf *s) {
    DEBUGASSERT(s);
    DEBUGASSERT(s->init == DYNINIT);
    DEBUGASSERT(!s->leng || s->bufr);
    return s->leng;
}