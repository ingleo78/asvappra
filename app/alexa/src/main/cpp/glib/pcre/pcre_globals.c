#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

#if defined _MSC_VER || defined  __SYMBIAN32__
static void* LocalPcreMalloc(size_t aSize) {
    return malloc(aSize);
}
static void LocalPcreFree(void* aPtr) {
    free(aPtr);
}
PCRE_EXP_DATA_DEFN int (*pcre_callout)(pcre_callout_block *) = NULL;
#elif !defined VPCOMPAT
PCRE_EXP_DATA_DEFN int   (*pcre_callout)(pcre_callout_block *) = NULL;
#endif