#include "curl_setup.h"

#ifdef NETWARE
#ifdef __NOVELL_LIBC__
int netware_init(void) { return 0; }
#else
#include <nwnspace.h>
#include <nwthread.h>
#include <nwadv.h>
#include <netdb.h>
NETDB_DEFINE_CONTEXT
#include <netinet/in.h>
#include <arpa/inet.h>

NETINET_DEFINE_CONTEXT

int netware_init(void) {
    int rc = 0;
    unsigned int myHandle = GetNLMHandle();
    void (*pUnAugmentAsterisk)(int) = (void(*)(int))ImportSymbol(myHandle, "UnAugmentAsterisk");
    void (*pUseAccurateCaseForPaths)(int) = (void(*)(int))ImportSymbol(myHandle, "UseAccurateCaseForPaths");
    if(pUnAugmentAsterisk) pUnAugmentAsterisk(1);
    if(pUseAccurateCaseForPaths) UseAccurateCaseForPaths(1);
    UnimportSymbol(myHandle, "UnAugmentAsterisk");
    UnimportSymbol(myHandle, "UseAccurateCaseForPaths");
    if((SetCurrentNameSpace(4) == 255)) rc = 1;
    if((SetTargetNameSpace(4) == 255)) rc = rc + 2;
    return rc;
}
int __init_environment(void) { return 0; }
int __deinit_environment(void) { return 0; }
#endif
#endif