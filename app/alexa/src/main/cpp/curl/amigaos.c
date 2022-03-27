#include "curl_setup.h"

#ifdef __AMIGA__
#include "amigaos.h"
#if defined(HAVE_PROTO_BSDSOCKET_H) && !defined(USE_AMISSL)
#include <amitcp/socketbasetags.h>
#endif
#ifdef __libnix__
#include <stabs.h>
#endif
#endif
#include "curl_memory.h"
#include "memdebug.h"

#ifdef __AMIGA__
#if defined(HAVE_PROTO_BSDSOCKET_H) && !defined(USE_AMISSL)
struct Library *SocketBase = NULL;
extern int errno, h_errno;
#ifdef __libnix__
void __request(const char *msg);
#else
#define __request(msg)       Printf(msg "\n\a")
#endif
void Curl_amiga_cleanup() {
    if (SocketBase) {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
    }
}
bool Curl_amiga_init() {
    if (!SocketBase) SocketBase = OpenLibrary("bsdsocket.library", 4);
    if (!SocketBase) {
        __request("No TCP/IP Stack running!");
        return FALSE;
    }
    if(SocketBaseTags(SBTM_SETVAL(SBTC_ERRNOPTR(sizeof(errno))), (ULONG) &errno, SBTM_SETVAL(SBTC_LOGTAGPTR), (ULONG) "curl", TAG_DONE)) {
        __request("SocketBaseTags ERROR");
        return FALSE;
    }
#ifndef __libnix__
    atexit(Curl_amiga_cleanup);
#endif
    return TRUE;
}
#ifdef __libnix__
ADD2EXIT(Curl_amiga_cleanup, -50);
#endif
#endif
#ifdef USE_AMISSL
void Curl_amiga_X509_free(X509 *a) { X509_free(a); }
#endif
#endif /* __AMIGA__ */

