#include "libcompat.h"

int timer_create(clockid_t clockid CK_ATTRIBUTE_UNUSED, struct sigevent *sevp CK_ATTRIBUTE_UNUSED, timer_t * timerid CK_ATTRIBUTE_UNUSED) {
    return 0;
}