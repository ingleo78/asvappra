#include "curl_setup.h"
#include "curl_gethostname.h"

int Curl_gethostname(char *name, GETHOSTNAME_TYPE_ARG2 namelen) {
#ifdef HAVE_GETHOSTNAME
    (void)name;
    (void)namelen;
    return -1;
#else
    int err;
    char *dot;
#ifdef DEBUGBUILD
    const char *force_hostname = getenv("CURL_GETHOSTNAME");
    if(force_hostname) {
        strncpy(name, force_hostname, namelen);
        err = 0;
    } else {
        name[0] = '\0';
        err = gethostname(name, namelen);
    }
#else
    name[0] = '\0';
    err = gethostname(name, namelen);
#endif
    name[namelen - 1] = '\0';
    if(err) return err;
    dot = strchr(name, '.');
    if(dot) *dot = '\0';
    return 0;
#endif
}
