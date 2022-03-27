#include <errno.h>
#include <assert.h>
#include "curl_setup.h"

#ifndef HAVE_INET_NTOP
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "inet_ntop.h"
#include "curl_printf.h"

#define IN6ADDRSZ       16
#define INADDRSZ         4
#define INT16SZ          2

static char *inet_ntop4 (const unsigned char *src, char *dst, size_t size) {
    char tmp[sizeof("255.255.255.255")];
    size_t len;
    DEBUGASSERT(size >= 16);
    tmp[0] = '\0';
    (void)msnprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", ((int)((unsigned char)src[0])) & 0xff, ((int)((unsigned char)src[1])) & 0xff,
                    ((int)((unsigned char)src[2])) & 0xff, ((int)((unsigned char)src[3])) & 0xff);
    len = strlen(tmp);
    if(len == 0 || len >= size) {
        errno = ENOSPC;
        return (NULL);
    }
    strcpy(dst, tmp);
    return dst;
}
#ifdef ENABLE_IPV6
static char *inet_ntop6 (const unsigned char *src, char *dst, size_t size) {
    char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
    char *tp;
    struct {
        long base;
        long len;
    } best, cur;
    unsigned long words[IN6ADDRSZ / INT16SZ];
    int i;
    memset(words, '\0', sizeof(words));
    for(i = 0; i < IN6ADDRSZ; i++) words[i/2] |= (src[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    cur.base  = -1;
    best.len = 0;
    cur.len = 0;
    for(i = 0; i < (IN6ADDRSZ / INT16SZ); i++) {
        if(words[i] == 0) {
            if(cur.base == -1) cur.base = i, cur.len = 1;
            else cur.len++;
        } else if(cur.base != -1) {
            if(best.base == -1 || cur.len > best.len) best = cur;
            cur.base = -1;
        }
    }
    if((cur.base != -1) && (best.base == -1 || cur.len > best.len)) best = cur;
    if(best.base != -1 && best.len < 2) best.base = -1;
    tp = tmp;
    for(i = 0; i < (IN6ADDRSZ / INT16SZ); i++) {
        if(best.base != -1 && i >= best.base && i < (best.base + best.len)) {
            if(i == best.base) *tp++ = ':';
            continue;
        }
        if(i != 0) *tp++ = ':';
        if(i == 6 && best.base == 0 && (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
            if(!inet_ntop4(src + 12, tp, sizeof(tmp) - (tp - tmp))) {
                errno = ENOSPC;
                return (NULL);
            }
            tp += strlen(tp);
            break;
        }
        tp += msnprintf(tp, 5, "%lx", words[i]);
    }
    if(best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ)) *tp++ = ':';
    *tp++ = '\0';
    if((size_t)(tp - tmp) > size) {
        errno = ENOSPC;
        return (NULL);
    }
    strcpy(dst, tmp);
    return dst;
}
#endif
char *Curl_inet_ntop(int af, const void *src, char *buf, size_t size) {
    switch(af) {
        case AF_INET: return inet_ntop4((const unsigned char *)src, buf, size);
    #ifdef ENABLE_IPV6
        case AF_INET6: return inet_ntop6((const unsigned char *)src, buf, size);
    #endif
        default:
            errno = EAFNOSUPPORT;
            return NULL;
    }
}
#endif