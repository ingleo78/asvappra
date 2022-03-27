#include <assert.h>
#include "curl_setup.h"
#ifndef CURL_DISABLE_FTP
#include "wildcard.h"
#include "llist.h"
#include "fileinfo.h"
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

#define ZERO_NULL 0
#define DEBUGASSERT(x) assert(x)

static void fileinfo_dtor(void *user, void *element) {
    (void)user;
    Curl_fileinfo_cleanup(element);
}
CURLcode Curl_wildcard_init(struct WildcardData *wc) {
    Curl_llist_init(&wc->filelist, fileinfo_dtor);
    wc->state = CURLWC_INIT;
    return CURLE_OK;
}
void Curl_wildcard_dtor(struct WildcardData *wc) {
    if(!wc) return;
    if(wc->dtor) {
        wc->dtor(wc->protdata);
        wc->dtor = ZERO_NULL;
        wc->protdata = NULL;
    }
    DEBUGASSERT(wc->protdata == NULL);
    Curl_llist_destroy(&wc->filelist, NULL);
    free(wc->path);
    wc->path = NULL;
    free(wc->pattern);
    wc->pattern = NULL;
    wc->customptr = NULL;
    wc->state = CURLWC_INIT;
}
#endif