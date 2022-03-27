#include "libcompat.h"

#undef realloc
void *realloc(void *p, size_t n);
void *rpl_realloc(void *p, size_t n) {
    if(n == 0) n = 1;
    if(p == 0) return malloc(n);
    return realloc(p, n);
}