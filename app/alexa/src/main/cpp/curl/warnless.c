#include <assert.h>
#include "curl_setup.h"
#include "config-dos.h"
#if defined(__INTEL_COMPILER) && defined(__unix__)
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#endif

#define BUILDING_WARNLESS_C 1
#include "warnless.h"
#define DEBUGASSERT(x) assert(x)
#define CURL_MASK_SCHAR  0x7F
#define CURL_MASK_UCHAR  0xFF
#if (SIZEOF_SHORT == 2)
#define CURL_MASK_SSHORT  0x7FFF
#define CURL_MASK_USHORT  0xFFFF
#elif (SIZEOF_SHORT == 4)
#define CURL_MASK_SSHORT  0x7FFFFFFF
#define CURL_MASK_USHORT  0xFFFFFFFF
#elif (SIZEOF_SHORT == 8)
#define CURL_MASK_SSHORT  0x7FFFFFFFFFFFFFFF
#define CURL_MASK_USHORT  0xFFFFFFFFFFFFFFFF
#else
#error "SIZEOF_SHORT not defined"
#endif
#if (SIZEOF_INT == 2)
#define CURL_MASK_SINT  0x7FFF
#define CURL_MASK_UINT  0xFFFF
#elif (SIZEOF_INT == 4)
#define CURL_MASK_SINT  0x7FFFFFFF
#define CURL_MASK_UINT  0xFFFFFFFF
#elif (SIZEOF_INT == 8)
#define CURL_MASK_SINT  0x7FFFFFFFFFFFFFFF
#define CURL_MASK_UINT  0xFFFFFFFFFFFFFFFF
#elif (SIZEOF_INT == 16)
#define CURL_MASK_SINT  0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#define CURL_MASK_UINT  0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
#else
#error "SIZEOF_INT not defined"
#endif
#if (SIZEOF_LONG == 2)
#define CURL_MASK_SLONG  0x7FFFL
#define CURL_MASK_ULONG  0xFFFFUL
#elif (SIZEOF_LONG == 4)
#define CURL_MASK_SLONG  0x7FFFFFFFL
#define CURL_MASK_ULONG  0xFFFFFFFFUL
#elif (SIZEOF_LONG == 8)
#define CURL_MASK_SLONG  0x7FFFFFFFFFFFFFFFL
#define CURL_MASK_ULONG  0xFFFFFFFFFFFFFFFFUL
#elif (SIZEOF_LONG == 16)
#define CURL_MASK_SLONG  0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFL
#define CURL_MASK_ULONG  0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFUL
#else
#error "SIZEOF_LONG not defined"
#endif
#if (SIZEOF_CURL_OFF_T == 2)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFF)
#elif (SIZEOF_CURL_OFF_T == 4)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFFFFFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFFFFFF)
#elif (SIZEOF_CURL_OFF_T == 8)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFFFFFFFFFFFFFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFFFFFFFFFFFFFF)
#elif (SIZEOF_CURL_OFF_T == 16)
#define CURL_MASK_SCOFFT  CURL_OFF_T_C(0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
#define CURL_MASK_UCOFFT  CURL_OFF_TU_C(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
#else
#error "SIZEOF_CURL_OFF_T not defined"
#endif
#if (SIZEOF_SIZE_T == SIZEOF_SHORT)
#define CURL_MASK_SSIZE_T  CURL_MASK_SSHORT
#define CURL_MASK_USIZE_T  CURL_MASK_USHORT
#elif (SIZEOF_SIZE_T == SIZEOF_INT)
#define CURL_MASK_SSIZE_T  CURL_MASK_SINT
#define CURL_MASK_USIZE_T  CURL_MASK_UINT
#elif (SIZEOF_SIZE_T == SIZEOF_LONG)
#define CURL_MASK_SSIZE_T  CURL_MASK_SLONG
#define CURL_MASK_USIZE_T  CURL_MASK_ULONG
#elif (SIZEOF_SIZE_T == SIZEOF_CURL_OFF_T)
#define CURL_MASK_SSIZE_T  CURL_MASK_SCOFFT
#define CURL_MASK_USIZE_T  CURL_MASK_UCOFFT
#else
#error "SIZEOF_SIZE_T not defined"
#endif
unsigned short curlx_ultous(unsigned long ulnum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(ulnum <= (unsigned long) CURL_MASK_USHORT);
    return (unsigned short)(ulnum & (unsigned long) CURL_MASK_USHORT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
unsigned char curlx_ultouc(unsigned long ulnum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(ulnum <= (unsigned long) CURL_MASK_UCHAR);
    return (unsigned char)(ulnum & (unsigned long) CURL_MASK_UCHAR);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
int curlx_ultosi(unsigned long ulnum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(ulnum <= (unsigned long) CURL_MASK_SINT);
    return (int)(ulnum & (unsigned long) CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
curl_off_t curlx_uztoso(size_t uznum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4310)
#endif
    DEBUGASSERT(uznum <= (size_t) CURL_MASK_SCOFFT);
    return (curl_off_t)(uznum & (size_t) CURL_MASK_SCOFFT);
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
    #pragma warning(pop)
#endif
}
int curlx_uztosi(size_t uznum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(uznum <= (size_t) CURL_MASK_SINT);
    return (int)(uznum & (size_t) CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
unsigned long curlx_uztoul(size_t uznum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
#if (SIZEOF_LONG < SIZEOF_SIZE_T)
    DEBUGASSERT(uznum <= (size_t) CURL_MASK_ULONG);
#endif
    return (unsigned long)(uznum & (size_t) CURL_MASK_ULONG);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
unsigned int curlx_uztoui(size_t uznum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
#if (SIZEOF_INT < SIZEOF_SIZE_T)
    DEBUGASSERT(uznum <= (size_t) CURL_MASK_UINT);
#endif
    return (unsigned int)(uznum & (size_t) CURL_MASK_UINT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
int curlx_sltosi(long slnum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(slnum >= 0);
#if (SIZEOF_INT < SIZEOF_LONG)
    DEBUGASSERT((unsigned long) slnum <= (unsigned long) CURL_MASK_SINT);
#endif
    return (int)(slnum & (long) CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
unsigned int curlx_sltoui(long slnum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(slnum >= 0);
#if (SIZEOF_INT < SIZEOF_LONG)
    DEBUGASSERT((unsigned long) slnum <= (unsigned long) CURL_MASK_UINT);
#endif
    return (unsigned int)(slnum & (long) CURL_MASK_UINT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
unsigned short curlx_sltous(long slnum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(slnum >= 0);
    DEBUGASSERT((unsigned long) slnum <= (unsigned long) CURL_MASK_USHORT);
    return (unsigned short)(slnum & (long) CURL_MASK_USHORT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
ssize_t curlx_uztosz(size_t uznum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(uznum <= (size_t) CURL_MASK_SSIZE_T);
    return (ssize_t)(uznum & (size_t) CURL_MASK_SSIZE_T);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
size_t curlx_sotouz(curl_off_t sonum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(sonum >= 0);
    return (size_t)(sonum & (curl_off_t) CURL_MASK_USIZE_T);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
int curlx_sztosi(ssize_t sznum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(sznum >= 0);
#if (SIZEOF_INT < SIZEOF_SIZE_T)
    DEBUGASSERT((size_t) sznum <= (size_t) CURL_MASK_SINT);
#endif
    return (int)(sznum & (ssize_t) CURL_MASK_SINT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
unsigned short curlx_uitous(unsigned int uinum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(uinum <= (unsigned int) CURL_MASK_USHORT);
    return (unsigned short) (uinum & (unsigned int) CURL_MASK_USHORT);
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
size_t curlx_sitouz(int sinum) {
#ifdef __INTEL_COMPILER
    #pragma warning(push)
    #pragma warning(disable:810)
#endif
    DEBUGASSERT(sinum >= 0);
    return (size_t) sinum;
#ifdef __INTEL_COMPILER
    #pragma warning(pop)
#endif
}
#ifdef USE_WINSOCK
int curlx_sktosi(curl_socket_t s) {
    return (int)((ssize_t) s);
}
curl_socket_t curlx_sitosk(int i) {
    return (curl_socket_t)((ssize_t) i);
}
#endif
#if defined(WIN32) || defined(_WIN32)
ssize_t curlx_read(int fd, void *buf, size_t count) {
    return (ssize_t)read(fd, buf, curlx_uztoui(count));
}
ssize_t curlx_write(int fd, const void *buf, size_t count) {
    return (ssize_t)write(fd, buf, curlx_uztoui(count));
}
#endif
#if defined(__INTEL_COMPILER) && defined(__unix__)
int curlx_FD_ISSET(int fd, fd_set *fdset) {
    #pragma warning(push)
    #pragma warning(disable:1469)
    return FD_ISSET(fd, fdset);
    #pragma warning(pop)
}
void curlx_FD_SET(int fd, fd_set *fdset) {
    #pragma warning(push)
    #pragma warning(disable:1469)
    FD_SET(fd, fdset);
    #pragma warning(pop)
}
void curlx_FD_ZERO(fd_set *fdset) {
    #pragma warning(push)
    #pragma warning(disable:593)
    FD_ZERO(fdset);
    #pragma warning(pop)
}
unsigned short curlx_htons(unsigned short usnum) {
#if (__INTEL_COMPILER == 910) && defined(__i386__)
    return (unsigned short)(((usnum << 8) & 0xFF00) | ((usnum >> 8) & 0x00FF));
#else
    #pragma warning(push)
    #pragma warning(disable:810)
    return htons(usnum);
    #pragma warning(pop)
#endif
}
unsigned short curlx_ntohs(unsigned short usnum) {
#if (__INTEL_COMPILER == 910) && defined(__i386__)
    return (unsigned short)(((usnum << 8) & 0xFF00) | ((usnum >> 8) & 0x00FF));
#else
    #pragma warning(push)
    #pragma warning(disable:810)
    return ntohs(usnum);
    #pragma warning(pop)
#endif
}
#endif