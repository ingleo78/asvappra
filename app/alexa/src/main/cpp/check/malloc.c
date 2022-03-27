#include "libcompat.h"

#undef malloc
void *malloc(size_t n);
void *rpl_malloc(size_t n) {
    if(n == 0) n = 1;
    return malloc(n);
}