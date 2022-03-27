#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre_internal.h"

int _pcre_ord2utf8(int cvalue, uschar *buffer) {
#ifdef SUPPORT_UTF8
    register int i, j;
    for (i = 0; i < _pcre_utf8_table1_size; i++) if (cvalue <= _pcre_utf8_table1[i]) break;
    buffer += i;
    for (j = i; j > 0; j--) {
        *buffer-- = 0x80 | (cvalue & 0x3f);
        cvalue >>= 6;
    }
    *buffer = _pcre_utf8_table2[i] | cvalue;
    return i + 1;
#else
    (void)(cvalue);
    (void)(buffer);
    return 0;
#endif
}